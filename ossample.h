/*
** SAP Open Server 
** ---------------
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
** 	This header file is used by the Open Server sample programs.
** 	It contains the prototypes for the utility routines defined in
**	utils.c
*/

/*
** Define OpenServer version information.
*/
#define	EX_SRV_VERSION	CS_CURRENT_VERSION

/*
** Generic Macros
*/
#ifndef MIN
#define MIN(a, b)	(((CS_INT)(a) < (CS_INT)(b)) ? (a) : (b))
#endif /* MIN */

/*
** Global Variables
*/
extern	CS_INT		Ctcflags;	/* Context ct_debug flags. */
extern	CS_INT		Conflags;	/* Connect ct_debug flags. */

/*
** Utility Routine Prototypes
*/
extern CS_RETCODE proc_args(
	CS_CONTEXT *cp,
	CS_INT argc,
	CS_CHAR **argv,
	CS_CHAR *sname,
	CS_INT slen,
	CS_CHAR	*sqlname,
	CS_INT sqllen
	);
extern CS_RETCODE CS_PUBLIC server_err_handler(
	CS_VOID *server,
	CS_VOID *sp,
	CS_INT errornum,
	CS_INT severity,
	CS_INT state,
	CS_INT oserrnum,
	CS_CHAR *errtext,
	CS_INT errtextlen,
	CS_CHAR *oserrtext,
	CS_INT oserrtextlen
	);
extern CS_RETCODE CS_PUBLIC cs_err_handler(CS_CONTEXT *cp, CS_CLIENTMSG *msg);
extern CS_VOID syntax_error(CS_CHAR *msg);
extern CS_VOID print_version(CS_CONTEXT *cp, CS_CHAR *snamep);
extern CS_RETCODE CS_PUBLIC stop_regproc(SRV_PROC *sproc);
extern CS_RETCODE CS_PUBLIC stop_srv(SRV_PROC *sproc);
extern void error_exit(CS_CHAR *reason);

