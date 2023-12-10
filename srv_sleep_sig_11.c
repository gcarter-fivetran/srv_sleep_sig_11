/*
** srv_sleep() signal 11 EXAMPLE PROGRAM
** ----------------------------------------
**
** Description
** -----------
**	Case ID: 1095381 / 2023
**	ComponentSDK (BC-SYB-SDK)
**  CustomerFiveTran (0001875547)
**  Created at05.12.2023, 14:26:39
**  Last update at09.12.2023, 20:44:59
**  ReporterFivetran Developers (S0024832874)
**
**	Apparently, CT-Lib makes calls to SRV-Lib. In particular, ct_results()
**  and ct_cancel() are examples.
**
**	In this example, after srv_run() is called, the main thread continues
**  to make CT-Lib calls resulting in the signal 11 by srv_sleep().
**
**  This example is based upon CT-Lib example program getsend.c.
**
** Server Tables
** -------------
**	This example will create a table with the following 
**	definition in a database created by the example:
**
**	create table xxx (i1 int, t text, f float, i2 int)
**
**	The example will insert a single row into the table.
**	The example will drop the database it created.
**
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <ctpublic.h>
#include <ospublic.h>
#include <ossample.h>
#include "example.h"
#include "exutils.h"
#include "srv_sleep_sig_11.h"

/*****************************************************************************
** 
** defines and globals used.
** 
*****************************************************************************/
/*
** Global names used in this module
*/
CS_CHAR *Ex_appname = "srv_sleep_sig_11";
CS_CHAR *Ex_dbname = "sampledb";
CS_CHAR *Ex_tabname = "sampletext";
CS_CHAR *Ex_server   = EX_SERVER;
CS_CHAR *Ex_username = EX_USERNAME;
CS_CHAR *Ex_password = EX_PASSWORD;

/*
** Local defines.
*/
#define	MY_LOGIN_MSG 	"User '%s' logged in with password '%s'."
#define	MY_LANG_MSG 	"Language handler called with string '%s'."
#define	INFO_MSG1	(CS_INT)5555
#define	INFO_MSG2	(CS_INT)6666

/*
** Define what text values that we will manipulate
*/
#define	EX_TXT_INIT_VALUE	"This is the initial value of the text field"
#define	EX_TXT_UPD1_VALUE	"This is the text value after the first update"
#define	EX_TXT_UPD2_VALUE	"The second update changed the text to this"

/*
** The maximum length of text that this example program may use
*/
#define EX_MAX_TEXT		255

/*
** Define a structure used in managing the text data
*/

typedef struct _text_data
{
	CS_IODESC	iodesc;		/* iodesc associated with text value */
	CS_TEXT		textbuf[EX_MAX_TEXT];	/* holds the value */
	CS_INT		textlen;	/* number of bytes in textbuf */
} TEXT_DATA;

/*
** Prototypes for routines in the example code.
*/
CS_STATIC CS_RETCODE CreateDatabase(
        CS_CONNECTION *connection1,
        CS_CONNECTION *connection2
	);
CS_STATIC CS_RETCODE RemoveDatabase(
        CS_CONNECTION *connection1,
        CS_CONNECTION *connection2
	);
CS_STATIC CS_RETCODE CreateTable(
        CS_CONNECTION *connection
	);
CS_STATIC CS_INT DoGetSend(
        CS_CONNECTION *connection1,
        CS_CONNECTION *connection2
        );
CS_STATIC CS_RETCODE RetrieveData(
        CS_CONNECTION *connection,
        TEXT_DATA *textdata
	);
CS_STATIC CS_RETCODE FetchResults(
        CS_COMMAND *cmd,
        TEXT_DATA *textdata
	);
CS_STATIC CS_RETCODE UpdateTextData(
        CS_CONNECTION *connection,
        TEXT_DATA *textdata,
	char *newdata
	);
CS_STATIC CS_RETCODE ProcessTimestamp(
        CS_COMMAND *cmd,
        TEXT_DATA *textdata
	);
CS_STATIC CS_VOID ValidateTxt(
        TEXT_DATA *textdata,
	char *valid_str
	);
CS_STATIC CS_VOID DisplayData(
        TEXT_DATA *textdata
	);
CS_STATIC CS_VOID done_error(
        SRV_PROC *sp
    );


/*
** main()
** 
** Purpose:
**	Entry point for example program.
** 
** Parameters:
**	None, argc and argv will not be used.
**
** Return:
** 	EX_EXIT_ERROR  or EX_EXIT_SUCCEED
**
*/
int
main(int argc, char *argv[])
{
	CS_CONTEXT	*context;
    SRV_SERVER *server;
	CS_CONNECTION	*connection1 = NULL;
	CS_CONNECTION	*connection2 = NULL;
	CS_RETCODE	retcode;
	
	EX_SCREEN_INIT();

	fprintf(stdout,"srv_sleep() signal 11 Example\n");
	fflush(stdout);

	/* 
	** Allocate a context structure and initialize Client-Library and Server-Library
	*/
	retcode = ex_init(&context, &server);
	if (retcode != CS_SUCCEED)
	{
		ex_panic("ex_init failed");
	}

    /*
    ** Start the Open Server.
    */
    {
        pthread_t thread;
        pthread_attr_t attr;
        char msgbuf[CS_MAX_CHAR + CS_MAX_CHAR];


        if((retcode = pthread_attr_init(&attr)) != 0) {
            sprintf(msgbuf, "Pthread routine %s failed for LTL server %s (return code: %d). errno = %d. Interpretation: %s.\n",
                    "pthread_attr_init", SERVER_NAME, retcode, errno, strerror(errno));
            ex_error(msgbuf);
        }

        if((retcode = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)) != 0) {
            sprintf(msgbuf, "Pthread routine %s failed for LTL server %s (return code: %d). errno = %d. Interpretation: %s.\n",
                    "pthread_attr_setdetachstate(PTHREAD_CREATE_DETACHED)", SERVER_NAME, retcode, errno, strerror(errno));
            ex_error(msgbuf);
        }

        if((retcode = pthread_create(&thread, &attr,
                          (void*)srv_run, NULL)) != 0) {
            sprintf(msgbuf, "Failed to start the Open Server %s (return code from pthread_create(srv_run): %d). errno = %d. Interpretation: %s.\n",
                    SERVER_NAME, retcode, errno, strerror(errno));
            ex_error(msgbuf);
        }

        if((retcode = pthread_attr_destroy(&attr)) != 0) {
            sprintf(msgbuf, "Pthread routine %s failed for LTL server %s (return code: %d). errno = %d. Interpretation: %s.\n",
                    "pthread_attr_destroy", SERVER_NAME, retcode, errno, strerror(errno));
            ex_error(msgbuf);
        }
    }

	/* 
	** Establish two connections. Connection1 is used to 
	** select data. Connection2 is used for doing updates.
	** Allocate the connection structure, set their properties, and  
	** establish the connections.
	*/
	retcode = ex_connect(context, &connection1, Ex_appname,
				Ex_username, Ex_password, Ex_server);
	if (retcode == CS_SUCCEED)
	{
		retcode = ex_connect(context, &connection2, Ex_appname,
					Ex_username, Ex_password, Ex_server);
	}

	/*
	** Create a database for the sample program and change to it. The
	** routine will use the global variable Ex_dbname, which is defined in
	** the sample header file.
	*/
	if (retcode == CS_SUCCEED)
	{
		retcode = CreateDatabase(connection1, connection2);
		if (retcode != CS_SUCCEED)
		{
			ex_error("getsend: ex_create_db() failed");
		}
	}

	/*
	** Create a table for the example.
	*/
	if (retcode == CS_SUCCEED)
	{
		retcode = CreateTable(connection1);
	}

	/*
	** Execute the routines for the getsend sample
	*/
	if (retcode == CS_SUCCEED)
	{
		retcode = DoGetSend(connection1, connection2);
	}

	/*
	** Remove the sample database.
	*/
	if (retcode == CS_SUCCEED)
	{
		retcode = RemoveDatabase(connection1, connection2);
	}

	/*
	** Deallocate the allocated structures, close the connection,
	** and exit Client-Library.
	*/
	if (connection1 != NULL)
	{
		retcode = ex_con_cleanup(connection1, retcode);
	}
	if (connection2 != NULL)
	{
		retcode = ex_con_cleanup(connection2, retcode);
	}
	
	if (context != NULL)
	{
		retcode = ex_ctx_cleanup(context, retcode);
	}

	return (retcode == CS_SUCCEED) ? EX_EXIT_SUCCEED : EX_EXIT_FAIL;
}

/*
** CreateDatabase()
**
** Type of function:
** 	getsend program internal api
**
** Purpose:
** 	This routine uses ex_create_db() to create a database. It then
** 	does a ex_use_db() to attach to it.
**
** Parameters:
** 	connection1	- Pointer to CS_CONNECTION structure.
** 	connection2	- Pointer to CS_CONNECTION structure.
**
** Return:
**	CS_SUCCEED if database was created.
**	Otherwise a Client-Library failure code.
*/
CS_STATIC CS_RETCODE 
CreateDatabase(CS_CONNECTION *connection1, CS_CONNECTION *connection2)
{
	CS_RETCODE	retcode;

	/*
	** create database
	*/
	retcode = ex_create_db(connection1, Ex_dbname);
	if (retcode != CS_SUCCEED)
	{
		ex_error("CreateDatabase: ex_create_db() failed");
		return retcode;
	}

	/*
	** point both connections to the new database.
	*/
	if ((retcode = ex_use_db(connection1, Ex_dbname)) != CS_SUCCEED)
	{
		ex_error("CreateDatabase: ex_use_db(Ex_dbname) failed");
		return retcode;
	}

	if ((retcode = ex_use_db(connection2, Ex_dbname)) != CS_SUCCEED)
	{
		ex_error("CreateDatabase: ex_use_db(Ex_dbname) failed");
		return retcode;
	}

	return retcode;
}

/*
** RemoveDatabase()
**
** Type of function:
** 	getsend program internal api
**
** Purpose:
** 	This routine uses ex_remove_db() to remove a database.
**
**	Note: The server will only let us drop a database if there are no
**	current users logged in and using it. Since multiple connections
**	have been established and are using the sampledb database, we need
**	to have them use master before the drop. This could be optimized by
**	first closing connection2, then do the remove of the database on
**	connection1.
**
** Parameters:
** 	connection1	- Pointer to CS_CONNECTION structure.
** 	connection2	- Pointer to CS_CONNECTION structure.
**
** Return:
**	CS_SUCCEED if database was removed.
**	Otherwise a Client-Library failure code.
*/
CS_STATIC CS_RETCODE 
RemoveDatabase(CS_CONNECTION *connection1, CS_CONNECTION *connection2)
{
	CS_RETCODE	retcode;

	if ((retcode = ex_use_db(connection1, "master")) != CS_SUCCEED)
	{
		ex_error("RemoveDatabase: ex_use_db(master) failed");
		return retcode;
	}

	if ((retcode = ex_use_db(connection2, "master")) != CS_SUCCEED)
	{
		ex_error("RemoveDatabase: ex_use_db(master) failed");
		return retcode;
	}

	retcode = ex_remove_db(connection1, Ex_dbname);
	if (retcode != CS_SUCCEED)
	{
		ex_error("getsend: ex_remove_db() failed");
		return retcode;
	}

	return retcode;
}

/*
** CreateTable()
**
** Type of function:
** 	getsend program internal api
**
** Purpose:
** 	This routine sets up the table for the getdata operation
**
** Parameters:
** 	connection	- Pointer to CS_CONNECTION structure.
**
** Return:
**	CS_SUCCEED if table was created.
**	Otherwise a Client-Library failure code.
*/
CS_STATIC CS_RETCODE 
CreateTable(CS_CONNECTION *connection)
{

	CS_RETCODE	retcode;
	CS_CHAR		*cmdbuf;
	
	/* 
	** Allocate the buffer for the command string.
	*/
	cmdbuf = (CS_CHAR *) malloc(EX_BUFSIZE);
	if (cmdbuf == (CS_CHAR *)NULL)
	{
		ex_error("CreateTable: malloc() failed");
		return CS_MEM_ERROR;
	}

	/*
	** If the table exists, drop it.
	*/
	sprintf(cmdbuf, "if exists (select name from sysobjects \
			where name = \"%s\") drop table %s",
					Ex_tabname, Ex_tabname);
        if ((retcode = ex_execute_cmd(connection, cmdbuf)) != CS_SUCCEED)
        {
                ex_error("CreateTable: ex_execute_cmd(drop table) failed");
		free (cmdbuf);
		return retcode;
	}
		
	/*
	** Set up and send the command to create the table.  
	*/
	sprintf(cmdbuf, "create table %s (i1 int, t text, f float, i2 int)",
				Ex_tabname);
        if ((retcode = ex_execute_cmd(connection, cmdbuf)) != CS_SUCCEED)
        {
                ex_error("CreateTable: ex_execute_cmd(drop table) failed");
		free (cmdbuf);
		return retcode;
	}

	/*
	** Insert an initial value into the table
	*/
	sprintf(cmdbuf, "insert %s values (35, '%s', 20.3, 50)",
			Ex_tabname, EX_TXT_INIT_VALUE);
        if ((retcode = ex_execute_cmd(connection, cmdbuf)) != CS_SUCCEED)
        {
                ex_error("CreateTable: ex_execute_cmd(insert table) failed");
	}
	free(cmdbuf);	
	return retcode;
}

/*
** DoGetSend()
**
** Type of function:
** 	getsend program internal api
**
** Purpose:
** 	This routine is the main driver for doing the getdata operation.
** 	It assumes that tha database and tables have been set up.
**
** Parameters:
** 	connection	- Pointer to CS_CONNECTION structure.
** 	connection	- Pointer to CS_CONNECTION structure.
**
** Return:
*/
CS_STATIC CS_RETCODE 
DoGetSend(CS_CONNECTION *connection1, CS_CONNECTION *connection2)
{
	CS_RETCODE	retcode;
	TEXT_DATA	textdata;

	/* 
	** Retrieve the data initially in the table and
	** get the descriptor for the text data.
	*/
	if ((retcode = RetrieveData(connection1, &textdata)) != CS_SUCCEED)
	{
                ex_error("DoGetSend: RetrieveData failed");
                return retcode;
	}

	/* 
	** Display the text data retrieved, validate it is what we expected
	*/	
	DisplayData(&textdata);
	ValidateTxt(&textdata, EX_TXT_INIT_VALUE);

	/*
	** update the table with new text, validate that it's ok.
	*/
	retcode = UpdateTextData(connection2, &textdata, EX_TXT_UPD1_VALUE);
	if (retcode != CS_SUCCEED)
	{
                ex_error("DoGetSend: UpdateTextData failed");
                return retcode;
	}

	if ((retcode = RetrieveData(connection1, &textdata)) != CS_SUCCEED)
	{
                ex_error("DoGetSend: RetrieveData failed");
                return retcode;
	}
	DisplayData(&textdata);
	ValidateTxt(&textdata, EX_TXT_UPD1_VALUE);

	/*
	** Do it again with another text value.
	*/
	retcode = UpdateTextData(connection2, &textdata, EX_TXT_UPD2_VALUE);
	if (retcode != CS_SUCCEED)
	{
                ex_error("DoGetSend: UpdateTextData failed");
                return retcode;
	}

	if ((retcode = RetrieveData(connection1, &textdata)) != CS_SUCCEED)
	{
                ex_error("DoGetSend: RetrieveData failed");
                return retcode;
	}
	DisplayData(&textdata);
	ValidateTxt(&textdata, EX_TXT_UPD2_VALUE);

	return retcode;
}

/*
** RetrieveData()
**
** Type of function:
** 	getsend program internal api
** 
** Purpose:
**	This function queries the database for all columns and rows in the
**	table defined in EX_tabname.
**
** Parameters:
**	connection	- Pointer to CS_CONNECTION structure.
**	textdata	- Pointer to a TEXT_DATA structure to fill.
**
** Return:
**	CS_SUCCEED if text was retrieved correctly.
**	Otherwise a Client-Library failure code.
*/
CS_STATIC CS_RETCODE
RetrieveData(CS_CONNECTION *connection, TEXT_DATA *textdata)
{
	CS_RETCODE	retcode;
	CS_INT		res_type;
	CS_CHAR         *cmdbuf;
	CS_COMMAND	*cmd;

	/* 
	** Allocate the buffer for the command string.
	*/
	cmdbuf = (CS_CHAR *) malloc(EX_BUFSIZE);
	if (cmdbuf == (CS_CHAR *)NULL)
	{
		ex_error("RetrieveData: malloc() failed");
		return (CS_FAIL);
	}

	/*
	** Allocate a command handle to send the select query with
	*/
        if ((retcode = ct_cmd_alloc(connection, &cmd)) != CS_SUCCEED)
        {
                ex_error("RetrieveData: ct_cmd_alloc() failed");
		free(cmdbuf);
                return retcode;
        }

	/*
	** Set up the command to select from the table.  
	*/
	sprintf(cmdbuf, "select * from %s", Ex_tabname);
	retcode = ct_command(cmd, CS_LANG_CMD, cmdbuf, CS_NULLTERM, CS_UNUSED);
	if (retcode != CS_SUCCEED)
	{
                ex_error("RetrieveData: ct_command() failed");
		free(cmdbuf);
		ct_cmd_drop(cmd);
                return retcode;
	}
	free(cmdbuf);

	/*
	** Send the command to the server 
	*/
	if (ct_send(cmd) != CS_SUCCEED)
	{
                ex_error("RetrieveData: ct_send() failed");
		ct_cmd_drop(cmd);
                return retcode;
	}
					
	/*
	** Process the results of the command
	*/
	while ((retcode = ct_results(cmd, &res_type)) == CS_SUCCEED)
	{
		switch ((int)res_type)
		{
		    case CS_ROW_RESULT:
			/* 
			** Handle the expected row results.
			*/
			retcode = FetchResults(cmd, textdata);
			if (retcode != CS_SUCCEED)
			{
				ex_error("RetrieveData: FetchResults() failed");
				/*
				** Something failed so cancel all results.
				*/
				ct_cancel(NULL, cmd, CS_CANCEL_ALL);
				ct_cmd_drop(cmd);
				return retcode;
			}
			break;

		    case CS_CMD_SUCCEED:
		    case CS_CMD_DONE:
			/*
			** This means that the command succeeded or is
			** finished.
			*/
			break;

		    case CS_CMD_FAIL:
			/*
			** The server encountered an error while
			** processing our command.
			*/
			ex_error("RetrieveData: ct_result() returned CS_CMD_FAIL");
			ct_cmd_drop(cmd);
			return CS_FAIL;

	    	   default:
			/*
			** We got something unexpected.
			*/
			ex_error("RetrieveData: ct_results() returned unexpected result typ");
			/*
			** Cancel all results.
			*/
			ct_cancel(NULL, cmd, CS_CANCEL_ALL);
			break;
		}
	}

	/*
	** We're done processing results. Let's check the
	** return value of ct_results() to see if everything
	** went ok.
	*/
	switch ((int)retcode)
	{
		case CS_END_RESULTS:
			/*
			** Everything went fine.
			*/
			retcode = CS_SUCCEED;
			break;

		case CS_FAIL:
			/*
			** ct_results() call failed.
			*/
			ex_error("RetrieveData: ct_results() failed");
			break;

		default:
			/*
			** We got an unexpected return value.
			*/
			ex_error("RetrieveData: ct_results() returned unexpected result");
			break;
	}

	/*
	** Drop our command structure.
	*/
	if ((retcode = ct_cmd_drop(cmd)) != CS_SUCCEED)
	{
		ex_error("RetrieveData: ct_cmd_drop() failed");
		return retcode;
	}

	return retcode;
}

/*
** FetchResults()
**
** Type of function:
** 	getsend program internal api
** 
** Purpose:
**	This function retrieves the results expected
**	for this example. It demonstrates retrieving
**	a text column intermixed with columns of 
**	other data types. 
**
** Parameters:
** 	cmd		- Pointer to CS_COMMAND structure.
**	textdata	- Pointer to a TEXT_DATA structure to fill.
**
** Return:
**	CS_SUCCEED if text was fetched correctly.
**	Otherwise a Client-Library failure code.
*/
CS_STATIC CS_RETCODE
FetchResults(CS_COMMAND *cmd, TEXT_DATA *textdata)
{
	CS_RETCODE	retcode;
	CS_DATAFMT	fmt;
	CS_INT		firstcol;
	CS_TEXT		*txtptr;
	CS_FLOAT	floatitem;
	CS_INT		count;
	CS_INT		len;

	/* 
	** All binds must be of columns prior to the columns
	** to be retrieved by ct_get_data().
	** To demonstrate this, bind the first column returned.
	*/
	memset(&fmt, 0, sizeof(CS_DATAFMT));
	fmt.datatype = CS_INT_TYPE;
	fmt.maxlength = sizeof(CS_INT);
	fmt.count = 1;
	fmt.format = CS_FMT_UNUSED;

	retcode = ct_bind(cmd, 1, &fmt, &firstcol, NULL, NULL);
	if (retcode != CS_SUCCEED)
	{
		ex_error("FetchResults: ct_bind() failed");
		return retcode;
	}

	/*
	** Retrieve and display the results.
	*/
	while(((retcode = ct_fetch(cmd, CS_UNUSED, CS_UNUSED, CS_UNUSED,
		&count)) == CS_SUCCEED) || (retcode == CS_ROW_FAIL) )
	{
		/*
		** Check for a recoverable error.
		*/
		if (retcode == CS_ROW_FAIL)
		{
			ex_error("FetchResults: ct_fetch() returned CS_ROW_FAIL");
			continue;
		}
		
		/*
		** Get the text data item in the second column.
		** Loop until we have all the data for this item.
		** The text used for this example could be retrieved
		** in one ct_get_data call, but data could be too large
		** for this to be the case. Instead, the data would have to
		** be retrieved in chunks. The example will retrieve
		** the text in 5 byte increments to demonstrate retrieving
		** data items in chunks.
		*/
		txtptr = textdata->textbuf;
		textdata->textlen = 0;
		do
		{
			retcode = ct_get_data(cmd, 2, txtptr, 5, &len);
			textdata->textlen += len;
			/*
			** Protect against overflowing the string buffer.
			*/
			if ((textdata->textlen + 5) > (EX_MAX_TEXT - 1))
			{
				break;
			}
			txtptr += len;
		} while (retcode == CS_SUCCEED);
		
		if (retcode != CS_END_ITEM)
		{
			ex_error("FetchResults: ct_get_data() failed");
			return retcode;
		}
		
		/* 
		** Retrieve the descriptor of the text data. It is  
		** available while retrieving results of a select query.
		** The information will be needed for later updates.
		*/
		retcode = ct_data_info(cmd, CS_GET,  2, &textdata->iodesc);
		if (retcode != CS_SUCCEED)
		{
			ex_error("FetchResults: cs_data_info() failed");
			return retcode;
		}
		
		/*
		** Get the float data item in the third column.
		*/
		retcode = ct_get_data(cmd, 3, (CS_VOID *)&floatitem,
					sizeof (floatitem), &len);
		if (retcode != CS_END_ITEM)
		{
			ex_error("FetchResults: ct_get_data() failed");
			return(retcode);
		}
		
		/*
		** When using ct_get_data to process results, it is not
		** required to get all the columns in the row. To illustrate
		** this, the last column of the result set is not retrieved.
		*/
	}
	
	/*
	** We're done processing rows. Check the
	** final return value of ct_fetch().
	*/
	if (retcode == CS_END_DATA)
	{
		/*
		** Succeeded in retrieving data.
		*/
		retcode = CS_SUCCEED;
	}
	else 
	{
		/*	
		** Something unexpected happened. 
		*/
		ex_error("FetchResults: ct_fetch() failed");
	}

	return retcode;
}

/*
** UpdateTextData()
**
** Type of function:
** 	getsend program internal api
** 
** Purpose:
** 	This function updates the text column
**	with the specified text, and calls ProcessResults()
**	to retrieve the new timestamp for the updated
**	text column.
** 
** Parameters:
**	connection	- Pointer to CS_CONNECTION structure.
**	textdata	- Pointer to a TEXT_DATA structure to fill.
**	newdata		- Pointer to new data to send.
**
** Returns:
**	CS_SUCCEED if text was updated correctly.
**	Otherwise a Client-Library failure code.
*/
CS_STATIC CS_RETCODE
UpdateTextData(CS_CONNECTION *connection, TEXT_DATA *textdata, char *newdata)
{
	CS_RETCODE	retcode;
	CS_INT		res_type;
	CS_COMMAND	*cmd;
	CS_INT		i;
	CS_TEXT		*txtptr;
	CS_INT		txtlen;

	/*
	** Allocate a command handle to send the text with
	*/
        if ((retcode = ct_cmd_alloc(connection, &cmd)) != CS_SUCCEED)
        {
                ex_error("UpdateTextData: ct_cmd_alloc() failed");
                return retcode;
        }

	/*
	** Inform Client-Library the next data sent will be 
	** used for a text or image update.
	*/
	if ((retcode = ct_command(cmd, CS_SEND_DATA_CMD, NULL, CS_UNUSED,
			CS_COLUMN_DATA)) != CS_SUCCEED)
	{
                ex_error("UpdateTextData: ct_command() failed");
		ct_cmd_drop(cmd);
                return retcode;
	}
		
	/*
	** Fill in the description information for the update
	** and send it to Client-Library.
	*/
	txtptr = (CS_TEXT *)newdata;
	txtlen = strlen(newdata);

	textdata->iodesc.total_txtlen = txtlen;
	textdata->iodesc.log_on_update = CS_TRUE;
	retcode = ct_data_info(cmd, CS_SET, CS_UNUSED, &textdata->iodesc);
	if (retcode != CS_SUCCEED)
	{
                ex_error("UpdateTextData: ct_data_info() failed");
		ct_cmd_drop(cmd);
		return retcode;
	}

	/*
	** Send the text one byte at a time. This is not the best thing to do 
	** for performance reasons, but does demonstrate the ct_send_data()
	** can handle arbitrary amounts of data.
	*/
	for (i = 0; i < txtlen; i++, txtptr++)
	{
		retcode = ct_send_data(cmd, txtptr, (CS_INT)1);
		if (retcode != CS_SUCCEED)
		{
			ex_error("UpdateTextData: ct_send_data() failed");
			ct_cmd_drop(cmd);
			return retcode;
		}
	}

	/*
	** ct_send_data() does writes to internal network buffers. To insure
	** that all the data is flushed to the server, a ct_send() is done.
	*/
	if ((retcode = ct_send(cmd)) != CS_SUCCEED)
	{
                ex_error("UpdateTextData: ct_send() failed");
		ct_cmd_drop(cmd);
                return retcode;
	}

	/*
	** Process the results of the command
	*/
	while ((retcode = ct_results(cmd, &res_type)) == CS_SUCCEED)
	{
		switch ((int)res_type)
		{
		    case CS_PARAM_RESULT:
			/*
			** Retrieve a description of the parameter data.
			** Only timestamp data is expected in this example.
			*/
			retcode = ProcessTimestamp(cmd, textdata);
			if (retcode != CS_SUCCEED)
			{
				ex_error("UpdateTextData: ProcessTimestamp() failed");
				/*
				** Something failed so cancel all results.
				*/
				ct_cancel(NULL, cmd, CS_CANCEL_ALL);
				ct_cmd_drop(cmd);
				return retcode;
			}
			break;

		    case CS_STATUS_RESULT:
			/*	
			** Not expecting CS_STATUS_RESULT in this example, 
			** but if received results will be pending. Therefore, 
			** cancel the current result set.
			*/
			retcode = ct_cancel(NULL, cmd, CS_CANCEL_CURRENT);
			if (retcode != CS_SUCCEED)
			{
				ex_error("UpdateTextData: ct_cancel() failed");
				ct_cmd_drop(cmd);
				return retcode;
			}
			break;

		    case CS_CMD_SUCCEED:
		    case CS_CMD_DONE:
			/*
			** This means that the command succeeded or is
			** finished.
			*/
			break;

		    case CS_CMD_FAIL:
			/*
			** The server encountered an error while
			** processing our command.
			*/
			ex_error("UpdateTextData: ct_results() returned CS_CMD_FAIL");
			break;

	    	   default:
			/*
			** We got something unexpected.
			*/
			ex_error("UpdateTextData: ct_results() returned unexpected result typ");
			/*
			** Cancel all results.
			*/
			ct_cancel(NULL, cmd, CS_CANCEL_ALL);
			break;
		}
	}

	/*
	** We're done processing results. Let's check the
	** return value of ct_results() to see if everything
	** went ok.
	*/
	switch ((int)retcode)
	{
		case CS_END_RESULTS:
			/*
			** Everything went fine.
			*/
			retcode = CS_SUCCEED;
			break;

		case CS_FAIL:
			/*
			** ct_results() call failed.
			*/
			ex_error("UpdateTextData: ct_results() failed");
			break;

		default:
			/*
			** We got an unexpected return value.
			*/
			ex_error("UpdateTextData: ct_results() returned unexpected result");
			break;
	}

	/*
	** Drop our command structure.
	*/
	if ((retcode = ct_cmd_drop(cmd)) != CS_SUCCEED)
	{
		ex_error("UpdateTextData: ct_cmd_drop() failed");
		return retcode;
	}

	return retcode;
}

/*
** ProcessTimestamp()
** 
** Purpose:
**	This function retrieves the new timestamp for
**	the updated text column into the CS_IODESC 
**	structure.
** 
** Parameters:
**	cmd		- Pointer to a CS_COMMAND structure.
**	textdata	- Pointer to a TEXT_DATA structure to fill.
**
** Returns:
**	CS_SUCCEED if text was updated correctly.
**	Otherwise a Client-Library failure code.
*/
CS_STATIC CS_RETCODE
ProcessTimestamp(CS_COMMAND *cmd, TEXT_DATA *textdata)
{
	CS_RETCODE	retcode;
	CS_INT		count;
	CS_DATAFMT	datafmt;

	retcode = ct_describe(cmd, 1, &datafmt);
	if (retcode != CS_SUCCEED)
	{
                ex_error("ProcessTimestamp: ct_describe() failed");
                return retcode;
	}
		
	/*
	** Check if the data is a timestamp. If so, save it
	** to the CS_IODESC structure for future text updates.
	*/
	if (!(datafmt.status & CS_TIMESTAMP))
	{
		/* 
		** Unexpected parameter data was received.
		*/
                ex_error("ProcessTimestamp: unexpected parameter data received");
                return CS_FAIL;
	}

	/*
	** Bind the timestamp field of the io descriptor
	** to assign the new timestamp from the parameter
	** results.
	*/
	datafmt.maxlength = sizeof(textdata->iodesc.timestamp);
	datafmt.format     = CS_FMT_UNUSED;
	if ((retcode = ct_bind(cmd, 1, &datafmt, (CS_VOID *)textdata->iodesc.timestamp,
						&textdata->iodesc.timestamplen,
						NULL)) != CS_SUCCEED)
	{
                ex_error("ProcessTimestamp: ct_bind() failed");
                return retcode;
	}

	/*
	** Retrieve the parameter result containing the timestamp.
	*/
	retcode = ct_fetch(cmd, CS_UNUSED, CS_UNUSED, CS_UNUSED, &count);
	if (retcode != CS_SUCCEED)
	{
                ex_error("ProcessTimestamp: ct_fetch() failed");
		return retcode;
	}

	/*
	** The timestamp was retrieved, so cancel the 
	** rest of the result set.
	*/
	retcode = ct_cancel(NULL, cmd, CS_CANCEL_CURRENT);
	if (retcode != CS_SUCCEED)
	{
                ex_error("ProcessTimestamp: ct_cancel() failed");
	}
	return retcode;
}

/*
** ValidateTxt()
**
** Type of function:
** 	getsend program internal api
** 
** Purpose:
**	This function validates that the retrieved
**	text is the value that was expected.
**
** Parameters:
**	textdata	- Pointer to a TEXT_DATA structure to use.
**	valid_str	- Pointer to the expected text.
**
** Return:
**	Nothing.
*/
CS_STATIC CS_VOID
ValidateTxt(TEXT_DATA *textdata, char *valid_str)	
{
	char buf[EX_MAX_TEXT + 1];
	
	/*
	** Add a null terminator at the end of the text for comparing.
	*/
	memcpy(buf, textdata->textbuf, textdata->textlen);
	buf[textdata->textlen] = '\0';
	if (strcmp(buf, valid_str) == 0)
	{
		fprintf(stdout, "The retrieved text is the expected value.\n");
	}
	else
	{
		fprintf(stdout, "The retrieved text is NOT the expected value\n");
	}
	fflush(stdout);
}

/*
** DisplayData()
**
** Type of function:
** 	getsend program internal api
** 
** Purpose:
**	This function displays the text data last retrieved from the database.
**
** Parameters:
**	textdata	- Pointer to a TEXT_DATA structure to use.
**
** Return:
**	Nothing.
*/

CS_STATIC CS_VOID
DisplayData(TEXT_DATA *textdata)	
{
	char buf[EX_MAX_TEXT + 1];
	
	/*
	** Add a null terminator at the end of the 
	** text for displaying the value.
	*/
	memcpy(buf, textdata->textbuf, textdata->textlen);
	buf[textdata->textlen] = '\0';
	fprintf(stdout, "\nThe text data retrieved is: \n\t%s\n", buf);
	fflush(stdout);
}

/*
** start_handler
**
** This routine is the SRV_START event handler for this application.
** It will install
** a registered procedure to stop the server.
*/
CS_RETCODE CS_PUBLIC
start_handler(SRV_SERVER *server)
{
    CS_CONTEXT	*cp;
    char		 msgbuf[CS_MAX_CHAR + CS_MAX_CHAR];

    //sleep(5);

    /*
    ** Retrieve the global context.
    */
    if ( cs_ctx_global(EX_SRV_VERSION, &cp) == CS_FAIL )
    {
        return CS_FAIL;
    }

    sprintf(msgbuf, "Server %s is started.\n", SERVER_NAME);
    srv_log(server, CS_TRUE, msgbuf, CS_NULLTERM);

    return stop_regproc(server);
}

/*
** connect_handler
**
** This routine is the SRV_CONNECT event handler this application uses.
** Here we get the connecting client thread's user name and password,
** and echo them back to the client via an informational message.
*/
CS_RETCODE CS_PUBLIC
connect_handler(SRV_PROC *sp)
{
    CS_CONTEXT	*cp;			/* Context structure. */
    CS_SERVERMSG	msg;			/* The message we'll send. */
    CS_CHAR		user[CS_MAX_NAME];	/* The client's user name. */
    CS_CHAR		pwd[CS_MAX_NAME];	/* The client's password. */
    CS_CHAR		sname[CS_MAX_NAME];	/* The server name. */
    CS_INT		slen;			/* The server name length. */
    CS_INT		ulen;			/* The user name length. */
    CS_INT		plen;			/* The password length. */

    /*
    ** Initialization.
    */
    srv_bzero(&msg, sizeof(msg));

    /*
    ** Get the CS_CONTEXT we're using.
    */
    if ( cs_ctx_global(EX_SRV_VERSION, &cp) == CS_FAIL )
    {
        /*
        ** An error was already raised.
        */
        done_error(sp);

        return CS_FAIL;
    }

    /*
    ** Get the name of the server.
    */
    if ( srv_props(cp, CS_GET, SRV_S_SERVERNAME, sname,
                   CS_MAX_NAME, &slen) == CS_FAIL )
    {
        /*
        ** An error was already raised.
        */
        done_error(sp);

        return CS_FAIL;
    }

    /*
    ** Get the client thread's user name.
    */
    if ( srv_thread_props(sp, CS_GET, SRV_T_USER, user,
                          CS_MAX_NAME, &ulen) == CS_FAIL )
    {
        /*
        ** An error was already raised.
        */
        done_error(sp);

        return CS_FAIL;
    }

    /*
    ** Get the client thread's password.
    */
    if (srv_thread_props(sp, CS_GET, SRV_T_PWD, pwd,
                         CS_MAX_NAME, &plen) == CS_FAIL )
    {
        /*
        ** An error was already raised.
        */
        done_error(sp);

        return CS_FAIL;
    }

    /*
    ** Null terminate the user and password strings.
    */
    user[ulen] = (CS_CHAR)'\0';
    pwd[plen] = (CS_CHAR)'\0';

    /*
    ** Initialize the message we're sending. We'll
    ** pick an arbitrary message number, and copy the message
    ** string into the CS_SERVERMSG structure.
    */
    msg.msgnumber = INFO_MSG1;

    /*
    ** Fill in the message text field.
    */
    (CS_VOID)sprintf(msg.text, MY_LOGIN_MSG, user, pwd);
    msg.textlen = strlen(msg.text);

    /*
    ** Fill in the server name field as well.
    */
    (CS_VOID)strncpy(msg.svrname, sname, slen);
    msg.svrnlen = slen;

    /*
    ** Send the message to the client.
    */
    msg.status = (CS_FIRST_CHUNK|CS_LAST_CHUNK);
    if ( (srv_sendinfo(sp, &msg, CS_TRAN_UNDEFINED) == CS_FAIL) )
    {
        /*
        ** An error was already raised.
        */
        done_error(sp);

        return CS_FAIL;
    }

    /*
    ** Send a done to complete the command.
    */
    if ( srv_senddone(sp,  SRV_DONE_FINAL,
                      CS_TRAN_COMPLETED, (CS_INT)0) == CS_FAIL )
    {
        /*
        ** An error was already raised.
        */
        return CS_FAIL;
    }

    /*
    ** All done.
    */
    return CS_SUCCEED;
}


/*
** lang_handler
** This routine is the SRV_LANGUAGE event handler. All we do here
** is get the incoming language string, and send it back to the
** client via an informational message.
*/
CS_RETCODE CS_PUBLIC
lang_handler(SRV_PROC *sp)
{
    CS_CONTEXT	*cp;			/* Context structure. */
    CS_SERVERMSG	msg;			/* The message we'll send. */
    CS_CHAR		sname[CS_MAX_NAME];	/* The server name. */
    CS_INT		slen;			/* The server name length. */
    CS_CHAR		*cmd;
    CS_INT		len;			/* the length of the message. */

    /*
    ** Initialization.
    */
    srv_bzero(&msg, sizeof(msg));


    /*
    ** Get the CS_CONTEXT structure.
    */
    if ( cs_ctx_global(EX_SRV_VERSION, &cp) == CS_FAIL )
    {
        /*
        ** An error was already raised.
        */
        done_error(sp);

        return CS_FAIL;
    }

    /*
    ** Get the name of the server.
    */
    if ( srv_props(cp, CS_GET, SRV_S_SERVERNAME, sname,
                   CS_MAX_NAME, &slen) == CS_FAIL )
    {
        /*
        ** An error was already raised.
        */
        done_error(sp);

        return CS_FAIL;
    }


    /*
    ** Get the length of the language string.
    */
    if ( (len = srv_langlen(sp)) == -1 )
    {
        /*
        ** An error was already raised.
        */

        done_error(sp);

        return CS_FAIL;
    }

    /*
    ** Allocate enough space to hold the language string.
    */
    if ( (cmd = (CS_CHAR *)srv_alloc(len + 1)) == (CS_CHAR *)NULL )
    {
        /*
        ** An error was already raised.
        */
        done_error(sp);

        return CS_FAIL;
    }


    /*
    ** Get the language string itself.
    */
    if ( srv_langcpy(sp, 0, -1, cmd) == -1 )
    {
        /*
        ** An error was already raised.
        */
        done_error(sp);

        return CS_FAIL;
    }

    /*
    ** We may want to truncate this string so that it
    ** fits in the message text buffer.
    */
    if ( len > (CS_INT)(CS_MAX_MSG - strlen(MY_LANG_MSG) ))
    {
        cmd[CS_MAX_MSG - strlen(MY_LANG_MSG)] = (CS_CHAR)'\0';
    }

    /*
    ** Place the message string in place.
    */
    (CS_VOID)sprintf(msg.text, MY_LANG_MSG, cmd);
    msg.textlen = strlen(msg.text);

    /*
    ** Let's make up a message number.
    */
    msg.msgnumber = INFO_MSG2;

    /*
    ** Fill in the server name field as well.
    */
    (CS_VOID)strncpy(msg.svrname, sname, slen);
    msg.svrnlen = slen;

    /*
    ** Send the message to the client.
    */
    msg.status = (CS_FIRST_CHUNK|CS_LAST_CHUNK);
    if ( srv_sendinfo(sp, &msg, CS_TRAN_UNDEFINED) == CS_FAIL )
    {
        /*
        ** An error was already raised.
        */
        done_error(sp);

        return CS_FAIL;
    }

    /*
    ** Let's clean up.
    */
    srv_free(cmd);

    /*
    ** And finally, send a done to complete the command.
    */
    srv_senddone(sp, SRV_DONE_FINAL, CS_TRAN_COMPLETED, (CS_INT)0);

    /*
    ** All done.
    */
    return CS_SUCCEED;
}

/*
** done_error
**
** This routine is used to send a done-error message to client threads
** when some client-originated login or command has failed.
**
*/

CS_STATIC CS_VOID
done_error(SRV_PROC *sp)
{

    /*
    ** All we need to do is send the done. If this fails,
    ** print an error to the screen and return.
    */
    if ( srv_senddone(sp, SRV_DONE_ERROR | SRV_DONE_FINAL,
                      CS_TRAN_COMPLETED, (CS_INT)0) == CS_FAIL )
    {
        (CS_VOID)fprintf(stderr, "lang: Failed to send a done!\n");
    }

    return;
}
