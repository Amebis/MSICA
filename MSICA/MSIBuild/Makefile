#
#    Copyright ("1991-2015") Amebis
#
#    This file is part of MSICA.
#
#    MSICA is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    MSICA is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with MSICA. If not, see <http://www.gnu.org/licenses/>.
#

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
ScheduledTask	Task	N					Identifier		Primary key, non-localized token.
ScheduledTask	DisplayName	N					Formatted		Task name.
ScheduledTask	Application	N					Formatted		Task application name.
ScheduledTask	Parameters	Y					Formatted		Task command line parameters.
ScheduledTask	Directory_	N			Directory	1	Identifier		Task working directory.
ScheduledTask	Flags	N					DoubleInteger		Task's flags to pass in IScheduledWorkItem::SetFlags() call.
ScheduledTask	Priority	N					DoubleInteger	32;64;128;256;16384;32768	Task's priority to pass in ITask::SetPriority() call.
ScheduledTask	User	Y					Formatted		Account name to run task as. Use empty string for system account.
ScheduledTask	Password	Y					Formatted		Account password to run task as. Use NULL for system account.
ScheduledTask	Author	Y					Formatted		Task author.
ScheduledTask	Description	Y					Formatted		Task description.
ScheduledTask	IdleMin	Y					Integer		A value that specifies how long, in minutes, the system must remain idle before the work item can run.
ScheduledTask	IdleDeadline	Y					Integer		A value that specifies the maximum number of minutes that the Task Scheduler will wait for the idle-time period.
ScheduledTask	MaxRuntime	N					DoubleInteger		Specifies the maximum run time (in milliseconds), for the task. This parameter may be set to -1 to specify an unlimited time.
ScheduledTask	Condition	Y					Condition		Optional expression which skips the task if evaluates to expFalse. If the expression syntax is invalid, the engine will terminate, returning iesBadActionData.
ScheduledTask	Component_	N			Component	1	Identifier		Component the task is part of.
ServiceConfigure	ServiceConfigure	N					Identifier		Primary key, non-localized token.
ServiceConfigure	Name	N					Formatted		Name of a service. /, \, comma and space are invalid
ServiceConfigure	StartType	N					DoubleInteger	-1;0;1;2;3;4	Start type: -1=don't change, 0=boot, 1=system, 2=auto, 3=demand, 4=disabled
ServiceConfigure	Control	N					Integer	0;2;3;4;5;7	Bitwise combination of: 0=no action, 1=wait to finish, 2=start, 4=stop
ServiceConfigure	Condition	Y					Condition		Optional expression which skips the service configuration if evaluates to expFalse. If the expression syntax is invalid, the engine will terminate, returning iesBadActionData.
ServiceConfigure	Sequence	Y							Number that determines the sort order in which the configurations are to be executed.
TaskTrigger	Trigger	N					Identifier		Primary key, non-localized token.
TaskTrigger	BeginDate	N					DoubleInteger		Date that the task trigger activates (counted in days from January 1st, 1980). The beginning date must be specified when setting a task.
TaskTrigger	EndDate	Y					DoubleInteger		Date that the task trigger deactivates (counted in days from January 1st, 1980).
TaskTrigger	StartTime	N					DoubleInteger		Time of the day the task runs (counted in minutes since midnight).
TaskTrigger	StartTimeRand	Y					Integer		Maximum random number of minutes to add to start time.
TaskTrigger	MinutesDuration	Y					DoubleInteger		Number of minutes after the task starts that the trigger will remain active. The number of minutes specified here must be greater than or equal to the MinutesInterval column.
TaskTrigger	MinutesInterval	Y					DoubleInteger		Number of minutes between consecutive task executions. This number is counted from the start of the previous scheduled task. The number of minutes specified here must be less than the MinutesDuration column.
TaskTrigger	Flags	N					DoubleInteger		Value that describes the behavior of the trigger. This value is a combination of the flags in TASK_TRIGGER::rgFlags.
TaskTrigger	Type	N	0	7			Integer		A TASK_TRIGGER_TYPE enumerated value that specifies the type of trigger. This member is used with Type. The type of trigger specified here determines which fields of the TRIGGER_TYPE_UNION specified in Type member will be used. Trigger type is based on when the trigger will run the task.
TaskTrigger	DaysInterval	Y					Integer		Specifies the number of days between task runs.
TaskTrigger	WeeksInterval	Y					Integer		Number of weeks between invocations of a task.
TaskTrigger	DaysOfTheWeek	Y	0	127			Integer		Value that describes the days of the week the task runs. This value is a bitfield and is a combination of the following values: 1=Sun, 2=Mon, 4=Tue, 8=Wed, 16=Thu, 32=Fri, 64=Sat.
TaskTrigger	DaysOfMonth	Y					DoubleInteger		Specifies the day of the month a task runs. This value is a bitfield that specifies the day(s) the task will run. Bit 0 corresponds to the first of the month, bit 1 to the second, and so forth.
TaskTrigger	WeekOfMonth	Y					Integer	1;2;3;4;5	Specifies the week of the month when the task runs. This value is exclusive and is one of the following: 1=The task will run between the first and seventh day of the month, 2=The task will run between the eighth and 14th day of the month, 3=The task will run between the 15th and 21st day of the month, 4=The task will run between the 22nd and 28th of the month, 5=The task will run between the last seven days of the month.
TaskTrigger	MonthsOfYear	Y	0	4095			Integer		Specifies the month(s) when the task runs. This value is a combination of the following values: 1=Jan, 2=Feb, 4=Mar, 8=Apr, 16=May, 32=Jun, 64=Jul, 128=Aug, 256=Sep, 512=Oct, 1024=Nov, 2048=Dec.
TaskTrigger	Task_	N			ScheduledTask	1	Identifier		Key of the Trigger's task.
WLANProfile	WLANProfile	N					Identifier		Primary key, non-localized token.
WLANProfile	Name	N					Formatted		Profile name
WLANProfile	Binary_	N			Binary	1	Identifier		The profile XML file. The binary information is looked up from the Binary Table.
WLANProfile	Condition	Y					Condition		Optional expression which skips the profile if evaluates to expFalse. If the expression syntax is invalid, the engine will terminate, returning iesBadActionData.
WLANProfile	Component_	N			Component	1	Identifier		Component the profile is part of.
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
RemoveScheduledTasksExec	Removing scheduled tasks	Scheduled task: [1]
StopServicesExec	Stopping services	Service: [1]
RemoveWLANProfilesExec	Removing WLAN profiles	WLAN profile: [1]
RemoveCertificatesExec	Removing certificates	Certificate: [1]
InstallCertificatesExec	Installing certificates	Certificate: [1]
InstallWLANProfilesExec	Installing WLAN profiles	WLAN profile: [1]
SetServiceStartExec	Configuring services	Service: [1]
StartServicesExec	Starting services	Service: [1]
InstallScheduledTasksExec	Installing scheduled tasks	Scheduled task: [1]
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
	"$(LANG).$(PLAT).$(CFG).Binary-1\MSICA.dll"

"$(LANG).$(PLAT).$(CFG).Binary-1.idt" : "Makefile" "..\..\..\include\MSIBuildCfg.mak" "$(LANG).$(PLAT).$(CFG).Binary-1"
	-if exist $@ del /f /q $@
	move /y << $@ > NUL
Name	Data
s$(MSIBUILD_LENGTH_ID)	v0
Binary	Name
binMSICA.dll	MSICA.dll
<<NOKEEP

"$(LANG).$(PLAT).$(CFG).Binary-1\MSICA.dll" : "..\..\output\$(PLAT).$(CFG)\MSICA.dll"
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
MSICAInitialize	1	binMSICA.dll	MSICAInitialize
RemoveScheduledTasksRollback	3329	binMSICA.dll	ExecuteSequence
RemoveScheduledTasksExec	3073	binMSICA.dll	ExecuteSequence
RemoveScheduledTasksCommit	3585	binMSICA.dll	ExecuteSequence
StopServicesRollback	3329	binMSICA.dll	ExecuteSequence
StopServicesExec	3073	binMSICA.dll	ExecuteSequence
StopServicesCommit	3585	binMSICA.dll	ExecuteSequence
RemoveWLANProfilesRollback	3329	binMSICA.dll	ExecuteSequence
RemoveWLANProfilesExec	3073	binMSICA.dll	ExecuteSequence
RemoveWLANProfilesCommit	3585	binMSICA.dll	ExecuteSequence
RemoveCertificatesRollback	3329	binMSICA.dll	ExecuteSequence
RemoveCertificatesExec	3073	binMSICA.dll	ExecuteSequence
RemoveCertificatesCommit	3585	binMSICA.dll	ExecuteSequence
InstallCertificatesRollback	3329	binMSICA.dll	ExecuteSequence
InstallCertificatesExec	3073	binMSICA.dll	ExecuteSequence
InstallCertificatesCommit	3585	binMSICA.dll	ExecuteSequence
InstallWLANProfilesRollback	3329	binMSICA.dll	ExecuteSequence
InstallWLANProfilesExec	3073	binMSICA.dll	ExecuteSequence
InstallWLANProfilesCommit	3585	binMSICA.dll	ExecuteSequence
SetServiceStartRollback	3329	binMSICA.dll	ExecuteSequence
SetServiceStartExec	3073	binMSICA.dll	ExecuteSequence
SetServiceStartCommit	3585	binMSICA.dll	ExecuteSequence
StartServicesRollback	3329	binMSICA.dll	ExecuteSequence
StartServicesExec	3073	binMSICA.dll	ExecuteSequence
StartServicesCommit	3585	binMSICA.dll	ExecuteSequence
InstallScheduledTasksRollback	3329	binMSICA.dll	ExecuteSequence
InstallScheduledTasksExec	3073	binMSICA.dll	ExecuteSequence
InstallScheduledTasksCommit	3585	binMSICA.dll	ExecuteSequence
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
MSICAInitialize		1510
RemoveScheduledTasksRollback		1850
RemoveScheduledTasksExec		1851
RemoveScheduledTasksCommit		1852
StopServicesRollback		1853
StopServicesExec		1854
StopServicesCommit		1855
RemoveWLANProfilesRollback		1856
RemoveWLANProfilesExec		1857
RemoveWLANProfilesCommit		1858
RemoveCertificatesRollback		1859
RemoveCertificatesExec		1860
RemoveCertificatesCommit		1861
InstallCertificatesRollback		6150
InstallCertificatesExec		6151
InstallCertificatesCommit		6152
InstallWLANProfilesRollback		6153
InstallWLANProfilesExec		6154
InstallWLANProfilesCommit		6155
SetServiceStartRollback		6156
SetServiceStartExec		6157
SetServiceStartCommit		6158
StartServicesRollback		6159
StartServicesExec		6160
StartServicesCommit		6161
InstallScheduledTasksRollback		6162
InstallScheduledTasksExec		6163
InstallScheduledTasksCommit		6164
<<NOKEEP


######################################################################
# ScheduledTask

All :: "$(LANG).$(PLAT).$(CFG).ScheduledTask-1.idt"

"$(LANG).$(PLAT).$(CFG).ScheduledTask-1.idt" : "Makefile" "..\..\..\include\MSIBuildCfg.mak"
	-if exist $@ del /f /q $@
	move /y << $@ > NUL
Task	DisplayName	Application	Parameters	Directory_	Flags	Priority	User	Password	Author	Description	IdleMin	IdleDeadline	MaxRuntime	Condition	Component_
s$(MSIBUILD_LENGTH_ID)	l255	s255	S255	s$(MSIBUILD_LENGTH_ID)	i4	i4	S255	S255	L255	L0	I2	I2	i4	S255	s$(MSIBUILD_LENGTH_ID)
ScheduledTask	Task
<<NOKEEP


######################################################################
# ServiceConfigure

All :: "$(LANG).$(PLAT).$(CFG).ServiceConfigure-1.idt"

"$(LANG).$(PLAT).$(CFG).ServiceConfigure-1.idt" : "Makefile" "..\..\..\include\MSIBuildCfg.mak"
	-if exist $@ del /f /q $@
	move /y << $@ > NUL
ServiceConfigure	Name	StartType	Control	Condition	Sequence
s$(MSIBUILD_LENGTH_ID)	l255	i4	i2	S255	I2
ServiceConfigure	ServiceConfigure
<<NOKEEP


######################################################################
# TaskTrigger

All :: "$(LANG).$(PLAT).$(CFG).TaskTrigger-1.idt"

"$(LANG).$(PLAT).$(CFG).TaskTrigger-1.idt" : "Makefile" "..\..\..\include\MSIBuildCfg.mak"
	-if exist $@ del /f /q $@
	move /y << $@ > NUL
Trigger	BeginDate	EndDate	StartTime	StartTimeRand	MinutesDuration	MinutesInterval	Flags	Type	DaysInterval	WeeksInterval	DaysOfTheWeek	DaysOfMonth	WeekOfMonth	MonthsOfYear	Task_
s$(MSIBUILD_LENGTH_ID)	i2	I2	i2	I2	I4	I4	i4	i2	I2	I2	I2	I4	I2	I2	s$(MSIBUILD_LENGTH_ID)
TaskTrigger	Trigger
<<NOKEEP


######################################################################
# WLANProfile

All :: "$(LANG).$(PLAT).$(CFG).WLANProfile-1.idt"

"$(LANG).$(PLAT).$(CFG).WLANProfile-1.idt" : "Makefile" "..\..\..\include\MSIBuildCfg.mak"
	-if exist $@ del /f /q $@
	move /y << $@ > NUL
WLANProfile	Name	Binary_	Condition	Component_
s$(MSIBUILD_LENGTH_ID)	l255	s$(MSIBUILD_LENGTH_ID)	S255	s$(MSIBUILD_LENGTH_ID)
WLANProfile	WLANProfile
<<NOKEEP


######################################################################
# Build MSM module!
######################################################################

!INCLUDE "..\..\..\MSIBuild\MSM.mak"
