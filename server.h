/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mfilipch <mfilipch@student.42.us.or>       +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2017/02/17 23:00:16 by mfilipch          #+#    #+#             */
/*   Updated: 2017/02/17 23:00:18 by mfilipch         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_H
# define SERVER_H
# define MAXBUF 1024
# include <netdb.h>
# include <strings.h>
# include <stdlib.h>
# include <stdio.h>
# include <unistd.h>
# include <sys/socket.h>
# include <signal.h>

typedef struct				s_env
{
	int						daemon;
	char					*name;
	const char				*port;
	int						sock_fd;
	int						nsocket;
	struct sockaddr_storage	cl_adr;
	socklen_t				adr_size;
	struct addrinfo			hints;
	struct addrinfo			*srvi;
	pid_t					pid;
}							t_env;
#endif
