#include <signal.h>

#include "Log.hpp"
#include "Server.hpp"

///////////////////////////////////////////////////////////////////////////////]
/** @brief Signal handler for SIGINT (Ctrl+C)
 *
 * This function is called automatically when the program receives the SIGINT signal
 * (typically triggered by pressing Ctrl+C in the terminal). Its main purpose is to
 * perform a graceful shutdown of the server.
 *
 * @param signum The signal number (ignored in this handler).
 * @param info   Additional signal information (ignored in this handler).
 * @param context Processor context at the time of signal (ignored in this handler).
 */
static void handle_sigint(int signum, siginfo_t *info, void *context)
{
	(void)info;
	(void)context;
	(void)signum;

	g_ServerEnd = true;
	LOG_WARNING(RED "CTRL-C recieved - Termination...");
	// exit(1);
}

///////////////////////////////////////////////////////////////////////////////]
/* Initializes signal handling for the program.
 *
 * Registers a custom handler for SIGINT (Ctrl+C) using sigaction.
 * The handler allows the server to shut down gracefully instead of
 * terminating immediately when an interrupt signal is received.
 *
 * Returns:
 *  0 on success
 *  1 on failure */
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
