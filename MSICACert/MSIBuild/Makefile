!INCLUDE "..\..\..\include\MSIBuildCfg.mak"

MSIBUILD_IS_LOCALIZEABLE=1


######################################################################
# _Validation

All :: "$(LANG).$(PLAT).$(CFG)._Validation-1.idt"

"$(LANG).$(PLAT).$(CFG)._Validation-1.idt" : "Makefile" "..\..\..\include\MSIBuildCfg.mak"
	-if exist $@ del /f /q $@
	move /y << $@ > NUL
Table	Column	Nullable	MinValue	MaxValue	KeyTable	KeyColumn	Category	Set	Description
s32	s32	s4	I4	I4	S255	I2	S32	S255	S255
_Validation	Table	Column
Certificate	Certificate	N					Identifier		Primary key, non-localized token.
Certificate	Store	N					Formatted	MY;Root;Trust;CA	Certificate store name for CertOpenStore() function call. The store provider is always CERT_STORE_PROV_SYSTEM. Therefore, possible values are: MY, Root, Trust, or CA.
Certificate	Flags	N					DoubleInteger		Flags for CertOpenStore() function call.
Certificate	Encoding	N					DoubleInteger	0;1;65536;65537	Certificate encoding (combination of X509_ASN_ENCODING(1) and PKCS_7_ASN_ENCODING(65536) flags).
Certificate	Binary_	N			Binary	1	Identifier		The certificate file. The binary information is looked up from the Binary Table.
Certificate	Condition	Y					Condition		Optional expression which skips the certificate if evaluates to expFalse. If the expression syntax is invalid, the engine will terminate, returning iesBadActionData.
Certificate	Component_	N			Component	1	Identifier		Component the certificate is part of.
<<NOKEEP


######################################################################
# ActionText

All :: "$(LANG).$(PLAT).$(CFG).ActionText-2.idt"

"En.$(PLAT).$(CFG).ActionText-2.idtx" : "Makefile" "..\..\..\include\MSIBuildCfg.mak"
	-if exist $@ del /f /q $@
	move /y << $@ > NUL
Action	Description	Template
s$(MSIBUILD_LENGTH_ID)	L0	L0
1252	ActionText	Action
InstallCertificates	Installing certificates	Certificate: [1]
<<NOKEEP

"De.$(PLAT).$(CFG).ActionText-2.idt" : "En.$(PLAT).$(CFG).ActionText-2.idtx" "..\L10N\de_DE.po"
	rcxgettext.exe idtp $@ $**

"En.$(PLAT).$(CFG).ActionText-2.idt" : "En.$(PLAT).$(CFG).ActionText-2.idtx"
	copy /y $** $@ > NUL

"It.$(PLAT).$(CFG).ActionText-2.idt" : "En.$(PLAT).$(CFG).ActionText-2.idtx" "..\L10N\it_IT.po"
	rcxgettext.exe idtp $@ $**

"Sl.$(PLAT).$(CFG).ActionText-2.idt" : "En.$(PLAT).$(CFG).ActionText-2.idtx" "..\L10N\sl_SI.po"
	rcxgettext.exe idtp $@ $**


######################################################################
# Binary

All :: \
	"$(LANG).$(PLAT).$(CFG).Binary-1.idt" \
	"$(LANG).$(PLAT).$(CFG).Binary-1\MSICACert.dll"

"$(LANG).$(PLAT).$(CFG).Binary-1.idt" : "Makefile" "..\..\..\include\MSIBuildCfg.mak" "$(LANG).$(PLAT).$(CFG).Binary-1"
	-if exist $@ del /f /q $@
	move /y << $@ > NUL
Name	Data
s$(MSIBUILD_LENGTH_ID)	v0
Binary	Name
binMSICACert.dll	MSICACert.dll
<<NOKEEP

"$(LANG).$(PLAT).$(CFG).Binary-1\MSICACert.dll" : "..\..\output\$(PLAT).$(CFG)\MSICACert.dll"
	copy /y $** $@ > NUL


######################################################################
# Certificate

All :: "$(LANG).$(PLAT).$(CFG).Certificate-1.idt"

"$(LANG).$(PLAT).$(CFG).Certificate-1.idt" : "Makefile" "..\..\..\include\MSIBuildCfg.mak"
	-if exist $@ del /f /q $@
	move /y << $@ > NUL
Certificate	Store	Flags	Encoding	Binary_	Condition	Component_
s$(MSIBUILD_LENGTH_ID)	s255	i4	i4	s$(MSIBUILD_LENGTH_ID)	S255	s$(MSIBUILD_LENGTH_ID)
Certificate	Certificate
<<NOKEEP


######################################################################
# CustomAction

All :: "$(LANG).$(PLAT).$(CFG).CustomAction-1.idt"

"$(LANG).$(PLAT).$(CFG).CustomAction-1.idt" : "Makefile" "..\..\..\include\MSIBuildCfg.mak"
	-if exist $@ del /f /q $@
	move /y << $@ > NUL
Action	Type	Source	Target
s$(MSIBUILD_LENGTH_ID)	i2	S$(MSIBUILD_LENGTH_ID)	S255
CustomAction	Action
EvaluateCertificates	1	binMSICACert.dll	EvaluateSequence
InstallCertificates	3073	binMSICACert.dll	ExecuteSequence
RollbackCertificates	3329	binMSICACert.dll	ExecuteSequence
CommitCertificates	3585	binMSICACert.dll	ExecuteSequence
<<NOKEEP


######################################################################
# InstallExecuteSequence

All :: "$(LANG).$(PLAT).$(CFG).InstallExecuteSequence-1.idt"

"$(LANG).$(PLAT).$(CFG).InstallExecuteSequence-1.idt" : "Makefile" "..\..\..\include\MSIBuildCfg.mak"
	-if exist $@ del /f /q $@
	move /y << $@ > NUL
Action	Condition	Sequence
s$(MSIBUILD_LENGTH_ID)	S255	I2
InstallExecuteSequence	Action
EvaluateCertificates		6577
RollbackCertificates		6576
InstallCertificates		6578
CommitCertificates		6579
<<NOKEEP


######################################################################
# Build MSM module!
######################################################################

!INCLUDE "..\..\..\MSIBuild\MSM.mak"
