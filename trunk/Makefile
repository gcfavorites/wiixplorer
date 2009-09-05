#---------------------------------------------------------------------------------
# Clear the implicit built in rules
#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPPC")
endif

include $(DEVKITPPC)/wii_rules

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
#---------------------------------------------------------------------------------
TARGET		:=	boot
BUILD		:=	build
SOURCES		:=	source source/libwiigui source/images source/fonts source/sounds source/network source/Prompts \
				source/BootHomebrew source/sevenzip source/libmad source/libgif source/libpngu source/FileStartUp \
				source/unzip source/Language source/mload source/usbstorage source/libbmp source/libtga
INCLUDES	:=	source

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------

CFLAGS		=	-g -O2 -Wall $(MACHDEP) $(INCLUDE)
CXXFLAGS	=	-save-temps -Xassembler -aln=$@.lst $(CFLAGS)
LDFLAGS		=	-g $(MACHDEP) -Wl,-Map,$(notdir $@).map,--section-start,.init=0x8090a000
#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
LIBS :=	-ljpeg -lpng -lz -lntfs -lfat -lwiiuse -lbte -lasnd -logc -ltremor -lfreetype -ltinysmb
#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:= $(CURDIR)

#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------
export PROJECTDIR 	:= $(CURDIR)
export OUTPUT		:=	$(CURDIR)/$(TARGETDIR)/$(TARGET)
export VPATH		:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
export DEPSDIR		:=	$(CURDIR)/$(BUILD)

#---------------------------------------------------------------------------------
# automatically build a list of object files for our project
#---------------------------------------------------------------------------------
SVNREV		:=	$(shell sh ./svnrev.sh)
export CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
export CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
sFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.S)))
TTFFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.ttf)))
PNGFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.png)))
OGGFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.ogg)))
PCMFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.pcm)))
ELFFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.elf)))
	
#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
	export LD	:=	$(CC)
else
	export LD	:=	$(CXX)
endif

export OFILES	:=	$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) \
					$(sFILES:.s=.o) $(SFILES:.S=.o) \
					$(TTFFILES:.ttf=.ttf.o) $(PNGFILES:.png=.png.o) \
					$(OGGFILES:.ogg=.ogg.o) $(PCMFILES:.pcm=.pcm.o) \
					$(addsuffix .o,$(ELFFILES)) 

#---------------------------------------------------------------------------------
# build a list of include paths
#---------------------------------------------------------------------------------
export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					-I$(CURDIR)/$(BUILD) \
					-I$(LIBOGC_INC)

#---------------------------------------------------------------------------------
# build a list of library paths
#---------------------------------------------------------------------------------
export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib) \
					-L$(LIBOGC_LIB)

export OUTPUT	:=	$(CURDIR)/$(TARGET)
.PHONY: $(BUILD) clean

#---------------------------------------------------------------------------------
$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
clean:
	@echo clean ...
	@rm -fr $(BUILD) $(OUTPUT).elf $(OUTPUT).dol

#---------------------------------------------------------------------------------
run:
	wiiload $(OUTPUT).dol

#---------------------------------------------------------------------------------
reload:
	wiiload -r $(OUTPUT).dol

#---------------------------------------------------------------------------------
else

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(OUTPUT).dol: $(OUTPUT).elf language
$(OUTPUT).elf: $(OFILES)
language: $(wildcard $(PROJECTDIR)/Languages/*.lang)

#---------------------------------------------------------------------------------
# This rule links in binary data with .ttf, .png, and .mp3 extensions
#---------------------------------------------------------------------------------
%.elf.o : %.elf
	@echo $(notdir $<)
	$(bin2o)

%.ttf.o : %.ttf
	@echo $(notdir $<)
	$(bin2o)

%.png.o : %.png
	@echo $(notdir $<)
	$(bin2o)
	
%.ogg.o : %.ogg
	@echo $(notdir $<)
	$(bin2o)
	
%.pcm.o : %.pcm
	@echo $(notdir $<)
	$(bin2o)


export PATH		:=	$(PROJECTDIR)/gettext-bin:$(PATH)

%.pot: $(CFILES) $(CPPFILES)
	@echo Updating Languagefiles ...
	@xgettext -C -cTRANSLATORS --from-code=utf-8 --sort-output --no-wrap --no-location -k -ktr -ktrNOOP -o $@ $^

%.lang: $(PROJECTDIR)/Languages/$(TARGET).pot
	@msgmerge -U -N --no-wrap --no-location --backup=none -q $@ $<
	@touch $@



-include $(DEPENDS)

#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------
