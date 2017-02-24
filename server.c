/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mfilipch <mfilipch@student.42.us.or>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/02/16 22:48:07 by mfilipch          #+#    #+#             */
/*   Updated: 2017/02/16 22:48:09 by mfilipch         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "server.h"

void	exit_call(const char *msg, int res, int sock_type, t_env *env)
{
	if (res > 0)
		perror(msg);
	else
		printf("%s", msg);
	if (sock_type == 1)
		close(env->sock_fd);
	else if (sock_type == 2)
		close(env->nsocket);
	else if (sock_type == 3)
	{
		close(env->sock_fd);
		close(env->nsocket);
	}
	exit(res);
}

void	serv_listen_loop(t_env *env, int len, char *pong)
{
	char	buf[MAXBUF + 1];
	int		res;
	pid_t	pid;


	pid = 0;
	//  Fork: parent to read from STDIN and accept new connections
	if (env->daemon == 0 && (pid = fork()) > 0)
	{
		while ((res = read(0, buf, MAXBUF)) > 0)
		{
			buf[res] = 0;
			printf("server input: %s", buf);
			if (!strcmp(buf, "quit\n"))
			{
				// Kills child that accpets connections
				// and runs as a daemon for open connections until all children "quit"
				close(env->sock_fd);
				printf("server: session is over\n");
				kill(pid, SIGKILL);
				system(env->name);
			}
		}
		(res < 0) ? exit_call("server: read error", 1, 1, env) : 0;
		(res == 0) ? exit_call("server: session is over\n", 0, 1, env) : 0;
	}
	// Fork: child to accept connections
	else if (pid == 0)
	{
		while (1)
		{
			if ((env->nsocket = accept(env->sock_fd,
				(struct sockaddr *)&(env->cl_adr), &(env->adr_size))) == -1)
				perror("accept");
			else if ((res = send(env->nsocket, pong, len, 0)) == -1)
				perror("send");
			// Fork: child to recieve/send messages from/to client 
			if (!(env->pid = fork()))
			{
				close(env->sock_fd);
				while ((res = recv(env->nsocket, buf, MAXBUF, 0)) > 0)
				{
					buf[res] = 0;
					printf("client: %s", buf);
					fflush(stdout);
					if (!strcmp(buf, "ping\n"))
					{
						if ((send(env->nsocket, pong, len, 0) == -1))
							exit_call("server: send error", 1, 2, env);
					}
					else if (!strcmp(buf, "quit\n"))
						exit_call("client: connection closed\n", 0, 2, env);
					else if (!strcmp(buf, "quitall\n"))
					{
						close(env->nsocket);
						printf("server: session is over\n");
						system(env->name);
					}
					else if (send(env->nsocket, "server: command is not recognized\n", 34, 0) == -1)
						exit_call("server: send error", 1, 2, env);
				}
				(res < 0) ? exit_call("server: recieve error", 1, 2, env) : 0;
				(res == 0) ? exit_call("client: connection closed\n", 0, 2, env) : 0;
			}
			// Fork: parent to continue accept connections
			else if (env->pid > 0)
				close(env->nsocket);
			else
				exit_call("server: fork error", 1, 3, env);
		}
	}
	else
		exit_call("server: fork error", 1, 2, env);
}

void	server_init(t_env *env)
{
	char	*pong;
	pid_t	pid;
	int		len;

	pong = "server: pong\n";
	len = strlen(pong);
	pid = 0;
	bzero(&(env->hints), sizeof(env->hints));
	env->hints.ai_family = AF_UNSPEC;
	env->hints.ai_socktype = SOCK_STREAM;
	env->hints.ai_flags = AI_PASSIVE;
	if (getaddrinfo(NULL, env->port, &(env->hints), &(env->srvi)))
		exit_call("getaddrinfo error", 1, 0, env);
	if ((env->sock_fd = socket(env->srvi->ai_family, env->srvi->ai_socktype,
		env->srvi->ai_protocol)) == -1)
		exit_call("server: socket", 1, 0, env);
	if (setsockopt(env->sock_fd, SOL_SOCKET,
		SO_REUSEADDR, &len, sizeof(int)) == -1)
		exit_call("server: port", 1, 0, env);
	if (bind(env->sock_fd, env->srvi->ai_addr, env->srvi->ai_addrlen) == -1)
		exit_call("server: bind", 1, 1, env);
	freeaddrinfo(env->srvi);
	if (env->daemon == 0 || !(pid = fork()))
	{
		if (listen(env->sock_fd, 5) == -1)
			exit_call("server: listen", 1, 1, env);
		env->adr_size = sizeof(env->cl_adr);
		serv_listen_loop(env, len, pong);
	}
	else if (pid < 0)
		exit_call("server: fork error", 1, 1, env);
	else
		exit_call("", 0, 1, env);
}

int		main(int ac, char const **av)
{
	t_env	env;

	//Check PORT and it's range
	if (ac < 2 || atoi(av[1]) <= 1024 || atoi(av[1]) >= 65536)
		exit_call("No valid port assigned", 0, 0, &env);
	env.port = av[1];
	// Store the name of the process to killall on server quit command
	env.name = malloc(strlen("killall ") + strlen(av[0]) - 1);
	env.name[strlen("killall ") + strlen(av[0]) - 2] = 0;
	strcpy(env.name, "killall ");
	strcat(env.name, &av[0][2]);
	// Set deamon flag
	env.daemon = 0;
	if (ac > 2 && !strcmp(av[2], "-D"))
		env.daemon = 1;	
	server_init(&env);
	return (0);
}
