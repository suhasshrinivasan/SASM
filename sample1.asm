COPY	START	4096  		.COPY FILE FROM INPUT TO OUTPUT		
FIRST	STL		RETADR		.SAVE RETURN ADDRESS
CLOOP	JSUB	RDREC		.READ INPUT RECORD
		LDA		LENGTH		.TEST FOR EOF
		COMP	ZERO
		JEQ		ENDFIL		.EXIT IF EOF FOUND
		JSUB	WRREC		.WRITE OUTPUT RECORD
		J 		CLOOP		.LOOP
ENDFIL  LDA		EOF			.INSERT EOF MARKER
		STA 	BUFFER		
		LDA 	THREE		.SET LENGTH = 3
		STA 	LENGTH
		JSUB	WRREC 		.WRITE EOF
		LDL 	RETADR 		.GET RETURN ADDRESS
		RSUB				.RETURN TO CALLER
EOF		BYTE 	C'EOF'
THREE	WORD 	3
ZERO 	WORD	0
RETADR 	RESW	1
LENGTH	RESW	1 			.LENGTH OF RECORD
BUFFER	RESB 	4096		.4096-BYTE BUFFER AREA
.
.		SUBROUTINE TO READ RECORD INTO BUFFER
.
RDREC	LDX		ZERO		.CLEAR LOOP COUNTER
		LDA		ZERO		.CLEAR A TO ZERO
RLOOP	TD		INPUT		.TEST INPUT DEV
		JEQ		RLOOP		.LOOP UNTIL READY
		RD 		INPUT		.READ CHARACTER INTO REGISTER
		COMP  	ZERO		.TEST FOR EOR
		JEQ 	EXIT		.EXIT LOOP IF EOR
		STCH	BUFFER,X	.STORE CHARACTER IN BUFFER
		TIX		MAXLEN		
		JLT 	RLOOP		.LOOP UNTIL MAX LENGTH
EXIT 	STX 	LENGTH		.SAVE REC LENGTH
		RSUB 				.RETURN TO CALLER
INPUT 	BYTE 	X'F1' 		.CODE FOR INPUT DEV
MAXLEN	WORD 	4096
.
.		SUBROUTINE TO WRITE RECORD FROM BUFFER
.
WRREC 	LDX 	ZERO		.CLEAR LOOP COUNTER
WLOOP 	TD  	OUTPUT 		.TEST OUTPUT DEV
		JEQ 	WLOOP 		.LOOP UNTIL READY
		LDCH 	BUFFER,X 	.GET CHARACTER FROM BUFFER
		WD 		OUTPUT 		.WRITE CHARACTER
		TIX 	LENGTH 		
		JLT 	WLOOP 		.LOOP UNTIL ALL CHARACTERS
		RSUB		
OUTPUT 	BYTE 	X'05'
		END 	FIRST
