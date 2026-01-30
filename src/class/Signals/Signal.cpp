#include <signal.h>
#include <iostream>
#include "_colors.h"
#include "Server.hpp"

static void handle_sigint(int signum, siginfo_t *info, void *context)
{
	(void)info;
	(void)context;
	(void)signum;
	g_ServerEnd = true;
	std::cout << RED << "\nSay goodbye to your cats buddy\n" << RESET;
}

int init_signals(void)
{
	struct sigaction sig;

	sigemptyset(&sig.sa_mask);
	sig.sa_sigaction = handle_sigint;
	sig.sa_flags = SA_SIGINFO;
	if (sigaction(SIGINT, &sig, NULL) == -1)
		return (1);
	return (0);
}
