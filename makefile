
#
# USAGE:
#       Just call "nmake"; no special options required.
#
# EXAMPLE:
#       nmake.exe /NOLOGO /S
#

#
# Some CL options:
#
#   /nologo - Suppresses display of sign-on banner
#   /c      - Compiles without linking
#   /Ox     - Uses maximum optimization (/Ob2gity /Gs)
#   /G7     - Optimizes code to favor the Pentium 4 or Athlon.
#   /Gy     - Separate functions for linker.
#   /Ze     - Enables language extensions
#   /Zi     - Enable debugging information
#   /Zl     - Removes default library name from .obj file
#   /TC     - Specifies a C source file
#   /FAc    - Machine and assembly code; .cod
#   /W3     - Sets warning level
#

#
# LINK opts:
#
#   /DLL    - Generates a DLL, adding *relocations* to the executable image.
#   /DEBUG  - :)
#

CC=$(CLBIN)\cl.exe
LNK=$(CLBIN)\link.exe
CCOPTS=/nologo /c /Ze /W3 /D_WIN32
LNKOPTS=/NOLOGO /NODEFAULTLIB:uuid.lib

#/D_WIN32_WINNT=0x500
#/NODEFAULTLIB:uuid.lib

# a directory for obj-files

ODIR=.\objs

# objs required for each module

SHARED_OBJS=\
	$(ODIR)\utils.obj\

LSAMOD_OBJS=\
	$(SHARED_OBJS)\
	$(ODIR)\lsamod.obj\
	$(ODIR)\lm_sam.obj\
	$(ODIR)\lm_pipe.obj\
	$(ODIR)\dout_so.obj\

LOADLL_OBJS=\
	$(SHARED_OBJS)\
	$(ODIR)\loadll.obj\
	$(ODIR)\dout_so.obj\

UT1_OBJS=\
	$(ODIR)\utils.obj\
	$(ODIR)\md5.obj\
	$(ODIR)\ut1.obj\
	$(ODIR)\dout_f.obj\

UT2_OBJS=\
	$(ODIR)\utils.obj\
	$(ODIR)\ut2.obj\
	$(ODIR)\dout_so.obj\

# standart rules

all : samsrv.dll lsamod.dll loadll.exe ut2.exe

re : clean all

clean :
	del $(ODIR)\*.obj
	del *.pdb *.ilk *.exp *.lib

# module rules

ut1.exe : $(UT1_OBJS)
	$(LNK) $(LNKOPTS) /ENTRY:main /OUT:ut1.exe $(UT1_OBJS) kernel32.lib user32.lib

ut2.exe : $(UT2_OBJS)
	$(LNK) $(LNKOPTS) /ENTRY:main /OUT:ut2.exe $(UT2_OBJS) kernel32.lib user32.lib

loadll.exe : $(LOADLL_OBJS)
	$(LNK) $(LNKOPTS) /ENTRY:main /OUT:loadll.exe $(LOADLL_OBJS) kernel32.lib user32.lib

nudge.exe : $(NUDGE_OBJS)
	$(LNK) $(LNKOPTS) /ENTRY:main /OUT:nudge.exe $(NUDGE_OBJS) kernel32.lib advapi32.lib user32.lib

lsamod.dll : $(LSAMOD_OBJS) .\lsamod.def
	$(LNK) $(LNKOPTS) /DLL /DEF:.\lsamod.def /OUT:lsamod.dll /ENTRY:DllMain $(LSAMOD_OBJS) kernel32.lib advapi32.lib user32.lib samsrv.lib

samsrv.dll : $(ODIR)\samsrv.obj .\samsrv.def
	$(LNK) $(LNKOPTS) /DLL /DEF:.\samsrv.def /OUT:samsrv.dll $(ODIR)\samsrv.obj


# inference rules

{.\ut}.c{$(ODIR)}.obj :
	$(CC) $(CCOPTS) /Fo$(ODIR)\ $<

{.\samsrv}.c{$(ODIR)}.obj :
	$(CC) $(CCOPTS) /Fo$(ODIR)\ $<

{.\lsamod2}.c{$(ODIR)}.obj :
	$(CC) $(CCOPTS) /Fo$(ODIR)\ $<

{.\lsamod}.c{$(ODIR)}.obj :
	$(CC) $(CCOPTS) /Fo$(ODIR)\ $<

{.\shared}.c{$(ODIR)}.obj :
	$(CC) $(CCOPTS) /Fo$(ODIR)\ $<

