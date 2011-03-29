noinst_LIBRARIES = libchardetect.a
libchardetect_a_CXXFLAGS = \
	-c \
	-O2 \
	-fno-rtti -fno-exceptions \
	-fvisibility=hidden -fvisibility-inlines-hidden \
	-nostdinc++ \
	-D_REENTRANT \
	-I$(srcdir)/src \
	-I$(srcdir)/include \
	-I$(srcdir)/src/tables
libchardetect_a_SOURCES = \
	src/CharDistribution.cpp \
	src/JpCntx.cpp \
	src/LangBulgarianModel.cpp \
	src/LangCyrillicModel.cpp \
	src/LangGreekModel.cpp \
	src/LangHebrewModel.cpp \
	src/LangHungarianModel.cpp \
	src/LangThaiModel.cpp \
	src/nsBig5Prober.cpp \
	src/nsCharSetProber.cpp \
	src/nsEscCharsetProber.cpp \
	src/nsEscSM.cpp \
	src/nsEUCJPProber.cpp \
	src/nsEUCKRProber.cpp \
	src/nsEUCTWProber.cpp \
	src/nsGB2312Prober.cpp \
	src/nsHebrewProber.cpp \
	src/nsLatin1Prober.cpp \
	src/nsMBCSGroupProber.cpp \
	src/nsMBCSSM.cpp \
	src/nsSBCharSetProber.cpp \
	src/nsSBCSGroupProber.cpp \
	src/nsSJISProber.cpp \
	src/nsUniversalDetector.cpp \
	src/nsUTF8Prober.cpp \
	src/entry/impl.cpp

noinst_HEADERS = \
	include/chardetect.h \
	src/CharDistribution.h \
	src/JpCntx.h \
	src/nsBig5Prober.h \
	src/nsCharSetProber.h \
	src/nsCodingStateMachine.h \
	src/nscore.h \
	src/nsEscCharsetProber.h \
	src/nsEUCJPProber.h \
	src/nsEUCKRProber.h \
	src/nsEUCTWProber.h \
	src/nsGB2312Prober.h \
	src/nsHebrewProber.h \
	src/nsLatin1Prober.h \
	src/nsMBCSGroupProber.h \
	src/nsPkgInt.h \
	src/nsSBCharSetProber.h \
	src/nsSBCSGroupProber.h \
	src/nsSJISProber.h \
	src/nsUniversalDetector.h \
	src/nsUTF8Prober.h \
	src/prmem.h \
	src/tables/Big5Freq.tab \
	src/tables/EUCKRFreq.tab \
	src/tables/EUCTWFreq.tab \
	src/tables/GB2312Freq.tab \
	src/tables/JISFreq.tab
