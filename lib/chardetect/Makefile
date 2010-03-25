TARGET=chardetect.a
CXXFLAGS=-c \
	-O2 \
	-fno-rtti -fno-exceptions \
	-fvisibility=hidden -fvisibility-inlines-hidden \
	-nostdinc++ \
	-D_REENTRANT -I./src -I./include

SUBDIRS=src
#DESTLIBDIR=/usr/lib/pcmanx/addons

all: $(TARGET)

$(TARGET):
	@for mdir in $(SUBDIRS); do \
	  if test -d $$mdir; then \
	    $(MAKE) -C $$mdir ; \
	  fi; \
	done
	libtool --mode=compile $(CXX) $(CXXFLAGS) src/entry/impl.cpp
	libtool --tag=CC --mode=link gcc -O2 -o $(TARGET) \
		impl.lo src/*.lo \
	        -avoid-version -module \
		-export-symbols src/entry/export-symbol.list \
		-lsupc++
	# libtool --tag=CC --mode=link gcc -O2 -o $(TARGET) \
	# 	impl.lo src/*.lo \
	#         -rpath $(DESTLIBDIR) -avoid-version -module \
	# 	-export-symbols src/entry/export-symbol.list \
	# 	-lsupc++
#	mv -f .libs/chardetect.so .

clean:
	@for mdir in $(SUBDIRS); do \
	  if test -d $$mdir; then \
	    $(MAKE) -C $$mdir clean ; \
	  fi; \
	done
	rm -f *.o *.lo *.la *.so *.a
	rm -rf .libs

check: all

install: 