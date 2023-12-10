/*
** SAP Open Server Sample
** ----------------------
**
** Copyright Notice and Disclaimer
** -------------------------------
**      (c) Copyright 2013.
**      SAP AG or an SAP affiliate company. All rights reserved.
**      Unpublished rights reserved under U.S. copyright laws.
**
**      SAP grants Licensee a non-exclusive license to use, reproduce,
**      modify, and distribute the sample source code below (the "Sample Code"),
**      subject to the following conditions:
**
**      (i) redistributions must retain the above copyright notice;
**
**      (ii) SAP shall have no obligation to correct errors or deliver
**      updates to the Sample Code or provide any other support for the
**      Sample Code;
**
**      (iii) Licensee may use the Sample Code to develop applications
**      (the "Licensee Applications") and may distribute the Sample Code in
**      whole or in part as part of such Licensee Applications, however in no
**      event shall Licensee distribute the Sample Code on a standalone basis;
**
**      (iv) and subject to the following disclaimer:
**      THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
**      INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
**      AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
**      SAP AG or an SAP affiliate company OR ITS LICENSORS BE LIABLE FOR ANY DIRECT, 
**	INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
**	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
**	SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
**	CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
**	OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
**      USE OF THE SAMPLE CODE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
**      DAMAGE.
**
** Description
** -----------
** 	This file contains utility routines used by the Open Server
** 	sample programs.
**
** 	The routines contained in this file are:
**
** 	server_err_handler()	An Open Server error handler routine.
** 	cs_err_handler()	A CS-Library error handler routine.
** 	proc_args()		A command-line argument parsing routine.
**	print_version()		A routine that prints Open Server version
**				information to stderr and to the log file.
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ospublic.h>
#include	<ctpublic.h>
#include	<oserror.h>
#include	<ossample.h>

CS_INT		Ctcflags;		/* Context ct_debug flags. */
CS_INT		Conflags;		/* Connect ct_debug flags. */

/*
** PROC_ARGS
**
** 	This routine parses the command line arguments passed to the
** 	Open Server application.
**
** 	The syntax expected is as follows:
**
** 	sample_name [server name] [-s ASE name] [-trace flags] 
*/
CS_RETCODE
proc_args(CS_CONTEXT *cp, CS_INT argc, CS_CHAR **argv, CS_CHAR *sname, CS_INT slen, 
	  CS_CHAR *sqlname, CS_INT sqllen)
{
	CS_INT		flags;			/* The trace types requested.*/
	CS_CHAR		mbuf[CS_MAX_MSG];	/* Message buffer. */

	/*
	** Initialization.
	*/
	flags = (CS_INT)0;
	*sname = '\0';
	Ctcflags = (CS_INT)0;
	Conflags = (CS_INT)0;

	if (sqlname != NULL)
	{
		*sqlname = '\0';
	}

	/*
	** If we have arguments, the first one should be the
	** Open Server name.
	*/
	if (argc > 1)
	{
		if (argv[1][0] == '-')
		{
			/*
			** We don't have a server name
			** argument, so just jump to the first flag.
			*/
			argv++;
		}
		else
		{
			/*
			** The first argument is the server name.
			*/
			(CS_VOID)strncpy(sname, argv[1], slen);
			
			/*
			** Skip to the second argument, if any.
			*/
			argv++;
			argv++;
		}
	}
	else
	{
		/*
		** We don't have any arguments, so we're all done.
		*/
		return CS_SUCCEED;
	}

	for (; *argv != NULL; argv++)
	{
		/*
		** Get the ASE name, if available.
		*/
		if (strcmp(*argv, "-s") == 0)
		{
			argv++;

			if (sqlname != NULL)
			{
				(CS_VOID)strncpy(sqlname, *argv, sqllen);
			}

			continue;
		}

		if (strcmp(*argv, "-") == 0)
		{
			argv++;
		}
		else
		{
			(*argv)++;
		}

		/*
		** TDS header tracing.
		*/
		if (strcmp(*argv, "h") == 0)
		{
			flags |= SRV_TR_TDSHDR;
		}
		/*
		** TDS data tracing.
		*/
		else if (strcmp(*argv, "d") == 0)
		{
			flags |= SRV_TR_TDSDATA;
		}
		/*
		** Attention tracing.
		*/
		else if (strcmp(*argv, "a") == 0)
		{
			flags |= SRV_TR_ATTN;
		}
		/*
		** Message queue tracing.
		*/
		else if (strcmp(*argv, "m") == 0)
		{
			flags |= SRV_TR_MSGQ;
		}
		/*
		** TDS token tracing.
		*/
		else if (strcmp(*argv, "t") == 0)
		{
			flags |= SRV_TR_TDSTOKEN;
		}
		/*
		** Event tracing.
		*/
		else if (strcmp(*argv, "e") == 0)
		{
			flags |= SRV_TR_EVENT;
		}
		/*
		** Deferred queue tracing.
		*/
		else if (strcmp(*argv, "q") == 0)
		{
			flags |= SRV_TR_DEFQUEUE;
		}
		/*
		** Net-Library tracing.
		*/
		else if (strcmp(*argv, "n") == 0)
		{
			flags |= SRV_TR_NETREQ;
			flags |= SRV_TR_NETDRIVER;
		}
		/*
		** CT-Lib: all debug flags.
		*/
		else if (strcmp(*argv, "c") == 0)
		{
			Conflags |= CS_DBG_ALL;
		}
		/*
		** CT-Lib API states.
		*/
		else if (strcmp(*argv, "f") == 0)
		{
			Ctcflags |= CS_DBG_API_STATES;
		}
		/*
		** CT-Lib asynchronous tracing.
		*/
		else if (strcmp(*argv, "y") == 0)
		{
			Ctcflags |= CS_DBG_ASYNC;
		}
		/*
		** CT-Lib diagnostic tracing.
		*/
		else if (strcmp(*argv, "g") == 0)
		{
			Conflags |= CS_DBG_DIAG;
		}
		/*
		** CT-Lib error tracing.
		*/
		else if (strcmp(*argv, "r") == 0)
		{
			Ctcflags |= CS_DBG_ERROR;
		}
		/*
		** CT-Lib memory tracing.
		*/
		else if (strcmp(*argv, "u") == 0)
		{
			Ctcflags |= CS_DBG_MEM;
		}
		/*
		** CT-Lib protocol tracing.
		*/
		else if (strcmp(*argv, "p") == 0)
		{
			Conflags |= CS_DBG_PROTOCOL;
		}
		/*
		** CT-Lib protocol state tracing.
		*/
		else if (strcmp(*argv, "v") == 0)
		{
			Conflags |= CS_DBG_PROTOCOL_STATES;
		}
		/*
		** CT-Lib network tracing.
		*/
		else if (strcmp(*argv, "w") == 0)
		{
			Ctcflags |= CS_DBG_NETWORK;
		}
		else
		{
			cs_snprintf(mbuf, sizeof(mbuf),
				"Unknown trace flag: '%s'.\n", *argv);
			syntax_error(mbuf);
			return CS_FAIL;
		}
	}

	/*
	** If there were any trace types set, activate them now.
	*/
	if (flags != (CS_INT)(0))
	{
		if (srv_props(cp, CS_SET, SRV_S_TRACEFLAG, &flags,
			sizeof(CS_INT), NULL) == CS_FAIL)
		{
			/*
			** An error was already raised.
			*/
			return CS_FAIL;
		}
	}

	/*
	** If CT-Lib context level tracing was set, we'll
	** activate it once ct_init() has been called.
	*/

	/*
	** All done.
	*/
	return CS_SUCCEED;
}

/*
** SERVER_ERR_HANDLER
**
** 	Open Server calls this routine whenever an internal error
** 	occurs. The arguments passed in describe the error number, severity
** 	and error string, as well as which thread, if any, was active when
** 	the error occurred.
**
** 	We deal with the different error severities as follows:
**
** 	SRV_FATAL_SERVER 	This error means that Open Server hit an
**				unrecoverable error. We try to log it and
**				return.
**
** 	SRV_FATAL_PROCESS	This severity indicates that a thread hit
**				an error that will cause the thread to 
**				terminate.  We log the error and return.
**
** 	SRV_INFO		This is an informational error. If the given
**				thread is an active client thread, we send the
**				error information that client via a message.
**
**  Parameters:
**	server		- The Open Server server running.
**	sp 		- The Open Server thread that got the error.
**	errornum 	- The Open Server error number.
**	severity 	- The error severity.
**	state		- The error state.
**	oserrnum 	- The operating system error number, if any.
**	errtext		- The text of the error message.
**	errtextlen 	- The length of the errtext message
**	oserrtext 	- The text of the operating system error message.
**	oserrtextlen 	- The length of the errtext message.
**
**  Returns:
**	CS_CONTINUE	Continue with this Open Server.
**	CS_FAIL		Exit the Open Server application.
*/
CS_RETCODE CS_PUBLIC 
server_err_handler(SRV_SERVER *server, SRV_PROC *sp, CS_INT errornum, CS_INT severity,
		   CS_INT state, CS_INT oserrnum, CS_CHAR *errtext, CS_INT errtextlen, 
		   CS_CHAR *oserrtext, CS_INT oserrtextlen)
{
	CS_CONTEXT	*cp;			/* Context structure. */
	CS_CHAR		sname[CS_MAX_NAME]; 	/* The server name. */
	CS_INT		slen;			/* The server name length. */
	CS_CHAR		mbuf[CS_MAX_MSG];	/* The message buffer. */
	CS_SERVERMSG	msg;			/* The message structure. */
	CS_INT		type;			/* The thread type. */
	CS_BOOL		client;			/* Is it a client thread? */
	CS_BOOL		iodead;			/* Is the thread alive?	*/

	/*
	** Initialization.
	*/
	srv_bzero(&msg, sizeof(msg));
	client = CS_FALSE;
	iodead = CS_FALSE;

	/*
	** Let's get the context this server uses.
	*/
	if (cs_ctx_global(EX_SRV_VERSION, &cp) == CS_FAIL)
	{
		/*
		** Can't raise an error, so just return.
		*/
		return CS_CONTINUE;
	}

	/*
	** Get the name of the server, if we were given
	** a SRV_SERVER structure in this call.
	*/
	if (server != NULL)
	{
		if (srv_props(cp, CS_GET, SRV_S_SERVERNAME, sname, 
				CS_MAX_NAME, &slen) == CS_FAIL)
		{
			/*
			** Can't raise an error, so just return.
			*/
			return CS_CONTINUE;
		}
		sname[slen] = '\0';
	}
	else
	{
		cs_strlcpy(sname, "no server", sizeof(sname));
	}

	/*
	** Is it an an operating system error? If so, log it.
	*/
	if (oserrnum != SRV_ENO_OS_ERR)
	{
		/*
		** Log the error.
		*/
		cs_snprintf(mbuf, sizeof(mbuf), "%s: OPERATING SYSTEM ERROR: %d: %s.\n",
			sname,  oserrnum, oserrtext);

		(CS_VOID)srv_log(server, CS_TRUE, mbuf, CS_NULLTERM);
		fprintf(stderr, "%s", mbuf);
		fflush(stderr);
	}

	/*
	** Is this a fatal error for the Open Server application?
	*/
	if (severity == SRV_FATAL_SERVER)
	{
		/*
		** Try to log the error, and return.
		*/
		cs_snprintf(mbuf, sizeof(mbuf), "%s: FATAL SERVER ERROR: %d/%d/%d: %s.\n",
			sname, errornum, severity, state, errtext);

		(CS_VOID)srv_log(server, CS_TRUE, mbuf, CS_NULLTERM);
		fprintf(stderr, "%s", mbuf);
		fflush(stderr);

		return SRV_EXIT_PROGRAM;
	}

	/*
	** Did a thread get a fatal error?
	*/
	if (severity == SRV_FATAL_PROCESS)
	{
		/*
		** Log the error, and return.
		*/
		cs_snprintf(mbuf, sizeof(mbuf),
			"%s: FATAL CONNECT ERROR: %d/%d/%d: %s.\n",
			sname, errornum, severity, state, errtext);
		(CS_VOID)srv_log(server, CS_TRUE, mbuf, CS_NULLTERM);
		fprintf(stderr, "%s", mbuf);
		fflush(stderr);
		
		return CS_CONTINUE;
	}

	/*
	** This is a non-fatal error or an informational message.
	**
	** If the error occurred on a client thread, send an
	** informational message containing the error information
	** to that client.
	*/
	if (sp != NULL)
	{
		/*
		** Get this thread's type.
		*/
		if (srv_thread_props(sp, CS_GET, SRV_T_TYPE,
			&type, sizeof(type), NULL) == CS_FAIL)
		{
			/*
			** Can't really do more here.
			*/
			return CS_FAIL;
		}

		/*
		** Check to see if we have a client thread.
		*/
		if ((type == SRV_TCLIENT) || (type == SRV_TSUBPROC))
		{
			client = CS_TRUE;
		}

		/*
		** Check to see if this client thread's I/O channel
		** is still active, before attempting to send a
		** message.
		*/
		if (srv_thread_props(sp, CS_GET, SRV_T_IODEAD,
			&iodead, sizeof(iodead), NULL) == CS_FAIL)
		{
			/*
			** Can't really do more here.
			*/
			return CS_FAIL;
		}

		/*
		** Now if this is an active client thread, send
		** the message.
		*/
		if (client && !iodead)
		{
			/*
			** Fill in the message information 
			** to be sent.
			*/
			msg.msgnumber = errornum;
			msg.state = state;
			msg.severity = severity;
			cs_strlcpy(msg.text, errtext, sizeof(msg.text));
			msg.textlen = strlen(msg.text);

			cs_strlcpy(msg.svrname, sname, sizeof(msg.svrname));
			msg.svrnlen = strlen(msg.svrname);
			
			/*
			** Send the message to the client.
			** We don't care if this fails, because
			** we want to log the error anyway.
			*/
			msg.status = (CS_FIRST_CHUNK|CS_LAST_CHUNK);
			(CS_VOID)srv_sendinfo(sp, &msg, CS_TRAN_UNDEFINED);
		}
	}

	/*
	** Let's log the error.
	*/
	cs_snprintf(mbuf, sizeof(mbuf), "%s: ERROR: %d/%d/%d: %s.\n", sname,
		errornum, severity, state, errtext);

	(CS_VOID)srv_log(server, CS_TRUE, mbuf, CS_NULLTERM);
	fprintf(stderr, "%s", mbuf);
	fflush(stderr);

	return CS_CONTINUE;
}

/*
** CS_ERR_HANDLER
**
** 	This routine is the CS-Library error handler used by this Open
** 	Server application. CS-Library calls it whenever an error occurs.
** 	Here we simply log the error and return.
*/
CS_RETCODE CS_PUBLIC
cs_err_handler(CS_CONTEXT *cp, CS_CLIENTMSG *msg)
{
	CS_CHAR		mbuf[CS_MAX_MSG*2];	/* Message buffer. */
	CS_CHAR		sname[CS_MAX_NAME]; 	/* The server name. */
	CS_INT		slen;			/* The server name length. */

	/*
	** Get the server name.
	*/
	if (srv_props(cp, CS_GET, SRV_S_SERVERNAME, sname, CS_MAX_NAME, &slen)
		== CS_FAIL)
	{
		/*
		** An error was already raised.
		*/
		return CS_SUCCEED;
	}
	sname[slen] = '\0';

	/*
	** Build the error message string to be logged.
	*/
	cs_snprintf(mbuf, sizeof(mbuf),  "%s: CS-Library error %d/%d/%d/%d - %s\n",
		sname, CS_LAYER(msg->msgnumber), 
		CS_ORIGIN(msg->msgnumber), CS_SEVERITY(msg->msgnumber), 
		CS_NUMBER(msg->msgnumber), msg->msgstring);

	/*
	** Log the error.
	*/
	(CS_VOID)srv_log(NULL, CS_TRUE, mbuf, CS_NULLTERM);

	/*
	** Log any operating system error information.
	*/
	if (msg->osstringlen > 0)
	{
		cs_snprintf(mbuf, sizeof(mbuf),
			"%s: CS-Library Operating system error %d - %s.\n",
			sname, msg->osnumber, msg->osstring);

		(CS_VOID)srv_log(NULL, CS_TRUE, mbuf,
			CS_NULLTERM);
	}

	/*
	** All done.
	*/
	return CS_SUCCEED;
}

/*
** SYNTAX_ERROR
**
** 	This routine is used to inform the user that a command-line
** 	syntax error occurred.
*/
CS_VOID	
syntax_error(CS_CHAR *msg)
{
	/*
	** Print the error string.
	*/
	(CS_VOID)fprintf(stderr, "Syntax error: %s\n", msg);
	(CS_VOID)fprintf(stderr, "Correct syntax is:\n\t");
	(CS_VOID)fprintf(stderr,
		"sample_name [server name] [-s ASE Name] \n");
	(CS_VOID)fprintf(stderr, "\t\t[-a (Attentions)] \n");
	(CS_VOID)fprintf(stderr, "\t\t[-c (CT-Lib all tracing)] \n");
	(CS_VOID)fprintf(stderr, "\t\t[-d (TDS data)] \n");
	(CS_VOID)fprintf(stderr, "\t\t[-e (Event)] \n");
	(CS_VOID)fprintf(stderr, "\t\t[-f (CT-Lib Function States)] \n");
	(CS_VOID)fprintf(stderr, "\t\t[-g (CT-Lib Diagnostics Tracing)] \n");
	(CS_VOID)fprintf(stderr, "\t\t[-h (TDS header)] \n");
	(CS_VOID)fprintf(stderr, "\t\t[-i (I/O)] \n");
	(CS_VOID)fprintf(stderr, "\t\t[-m (Msg Queue)] \n");
	(CS_VOID)fprintf(stderr, "\t\t[-n (Net-Library)] \n");
	(CS_VOID)fprintf(stderr, "\t\t[-p (CT-Lib Protocol Tracing)] \n");
	(CS_VOID)fprintf(stderr, "\t\t[-q (Deferred Queue)] \n");
	(CS_VOID)fprintf(stderr, "\t\t[-r (CT-Lib Error Tracing)] \n");
	(CS_VOID)fprintf(stderr, "\t\t[-t (TDS token)] \n");
	(CS_VOID)fprintf(stderr, "\t\t[-u (CT-Lib Memory Tracing)] \n");
	(CS_VOID)fprintf(stderr,
		"\t\t[-v (CT-Lib Protocol States Tracing)] \n");
	(CS_VOID)fprintf(stderr, "\t\t[-w (CT-Lib Network Tracing)] \n");
	(CS_VOID)fprintf(stderr, "\t\t[-y (CT-Lib Async Tracing)] \n");
	return;
}

/*
** PRINT_VERSION
**
**	This routine prints the version string to the log file and to
**	stdout.
**
** Parameters:
**	cp	Open Server context.
**	snamep	The Open Server application name.
**
** Returns:
**	Nothing.
*/
CS_VOID	
print_version(CS_CONTEXT *cp, CS_CHAR *snamep)
{
	CS_CHAR		buf[1024];
	CS_INT		outlen;
	CS_INT		len;

	/*
	** Get the version string for this Open Server application.
	** If this call fails we just return to the caller.  We don't
	** treat this as a fatal error.
	*/
	if (srv_props(cp, CS_GET, SRV_S_VERSION, buf, sizeof(buf), &outlen)
		== CS_FAIL)
	{
		return;
	}

	/*
	** Determine the offset for the null termination byte.  We need
	** to null terminate this string because we are going to use
	** fprintf to print it to stderr
	*/
	if (outlen >= sizeof(buf))
	{
		len = (sizeof(buf) - 1);
	}
	else
	{
		len = outlen;
	}

	/*
	** Null terminate the version string.
	*/
	buf[len] = '\0';

	/*
	** Write the version string to stderr and to the log file.
	*/
	(CS_VOID)fprintf(stderr, "%s: %s\n", snamep, buf);
	(CS_VOID)srv_log(NULL, CS_TRUE, buf, CS_NULLTERM);
	(CS_VOID)srv_log(NULL, CS_FALSE, "\n", CS_NULLTERM);
	return;
}

CS_RETCODE CS_PUBLIC
stop_regproc(SRV_SERVER *server)
{
	CS_INT          info;
	SRV_PROC        *sproc;

	/*
	**	Create a temporary SRV_PROC to register the procedure
	*/
	sproc = srv_createproc(server);
	if (sproc == NULL)
	{
		return CS_FAIL;
	}

	if (srv_regdefine(sproc, "stop_srv", CS_NULLTERM, stop_srv)
		== CS_FAIL)
	{
		return CS_FAIL;
	}

	if (srv_regcreate(sproc, &info) == CS_FAIL)
	{
		return CS_FAIL;
	}

	(CS_VOID)srv_termproc(sproc);

	return CS_SUCCEED;
}

/*
** STOP_SRV
**
**	This routine queues a SRV_STOP event to stop the Open Server.
**	If this fails, it just exits.
**
** Parameters:
**	spp	Thread control structure
**
** Returns:
**	CS_SUCCEED	Stop event was queued successfully
*/
CS_RETCODE CS_PUBLIC
stop_srv(SRV_PROC *spp)
{
	(CS_VOID)srv_senddone(spp, SRV_DONE_FINAL, 0, 0);

	if (srv_event(spp, SRV_STOP, NULL) != CS_SUCCEED)
	{
		(CS_VOID)fprintf(stderr,
			"Unable to queue a SRV_STOP event. Aborting...\n");
		exit(1);
	}
	return CS_SUCCEED;
}

/*
** Simple utility function to log a fatal error and exit.
*/
void
error_exit(CS_CHAR *reason)
{
	(CS_VOID)fprintf(stderr, "Fatal error: %s\n", reason);
	exit(1);
}
