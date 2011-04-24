%global elfname osd-lyrics

Name:           osdlyrics
Version:        0.3.20100604
Release:        2%{?dist}
Summary:        An OSD lyric show compatible with various media players

Group:          Applications/Multimedia
License:        GPLv3
URL:            http://code.google.com/p/osd-lyrics/
Source0:        http://osd-lyrics.googlecode.com/files/%{name}-%{version}.tar.gz
BuildRoot:      %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)
Patch0:         osdlyrics-0.3.20100604-desktop.patch
# Use our CXXFLAGS to properly generate debuginfo
Patch1:         osdlyrics-0.3.20100604-chardetect-cxxflags.patch
BuildRequires:  gtk2-devel, dbus-glib-devel, libcurl-devel, sqlite-devel
BuildRequires:  libnotify-devel, xmms2-devel, libmpd-devel, gettext-devel
BuildRequires:  libtool
#Requires:       gtk2, dbus-glib, libcurl, libglade2

%description
Osd-lyrics is a third-party lyrics display program,
and focus on OSD lyrics display.

%prep
%setup -q
%patch0 -p0
%patch1 -p0

%build
aclocal
autoconf
automake --add-missing
%configure
make %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT
make install DESTDIR=$RPM_BUILD_ROOT

%find_lang %{elfname}

%clean
rm -rf $RPM_BUILD_ROOT

%post
touch --no-create %{_datadir}/icons/hicolor &>/dev/null || :

%postun
if [ $1 -eq 0 ] ; then
    touch --no-create %{_datadir}/icons/hicolor &>/dev/null || :
    gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :
fi

%posttrans
gtk-update-icon-cache %{_datadir}/icons/hicolor &>/dev/null || :

%files -f %{elfname}.lang
%defattr(-,root,root,-)
%doc AUTHORS COPYING ChangeLog NEWS* README*
%{_bindir}/%{elfname}
%{_datadir}/applications/%{elfname}.desktop
%{_datadir}/icons/hicolor/*/*/%{elfname}*
%{_datadir}/%{elfname}/


%changelog
* Sun Jun 13 2010 Robin Lee <robinlee.sysu@gmail.com> - 0.3.20100604-2
- Spec file massive rewritten
- Updated to 0.3.20100604

* Wed Jun 09 2010 Liang Suilong <liangsuilong@gmail.com> 0.3.20100604-1
- Add Juk and Qmmp support
- Add app indicator support for Ubuntu 10.04
- Add singleton detection
- Honor MPD_HOST and MPD_PORT environment variables for MPD
- The `mouse click through' feature is back for GTK+ 2.20 users
- The appearance under a window manager without compositing support is correct now
- It won't crash now when you open the lyric assign dialog more than once
- The first line of lyric will not be lost when there is a BOM of utf-8 in the file.
- Fix that the last lyric doesn't get its progress with Rhythmbox.

* Wed Mar 31 2010 Liang Suilong <liangsuilong@gmail.com> 0.3.20100330-1
- Download lyrics from MiniLyrics
- Player control on background panel of OSD Window
- Encoding detection of LRC files
- Display player icon in notification
- FIX: Can not hide OSD Window
- FIX: Advance/delay offset doesn't work from popup menu
- Some minor fixes
- Drop amarok-1.4 support

* Fri Feb 12 2010 Liang Suilong <liangsuilong@gmail.com> 0.3.20100212-1
- Choose which lyric to download if there are more than one candidates
- Search lyrics manually
- Adjust lyric delay
- Support Quod Libet
- Display track infomation on tooltip of the trayicon
- Show notification of track infomation on track change
- Launch prefered player if no supported player is running
- A more graceful background on OSD Window
- Use themeable icons
- FIX: Crashes when hiding OSD Window under some distribution

* Tue Feb 02 2010 Liang Suilong <liangsuilong@gmail.com> 0.2.20100201-1
- FIX The program will not crash when DNS lookup timeout on searching or downloading lyrics

* Tue Feb 02 2010 Liang Suilong <liangsuilong@gmail.com> 0.2.20100201-1
- FIX The program will not crash when DNS lookup timeout on searching or downloading lyrics

* Sat Jan 09 2010 Liang Suilong <liangsuilong@gmail.com> 0.2.20100109-1
- Add MOC support
- Fix dowloading fails when title or artist is not set

* Wed Dec 30 2009 Liang Suilong <liangsuilong@gmail.com> 0.2.20091227-1
- Add mpd support
- Add BR: libmpd-devel
- Enable Amarok 1.4 support

* Tue Sep 22 2009 Liang Suilong <liangsuilong@gmail.com> 0.2.20090919-2
- Add gettext-devel as BuildRequires

* Sat Sep 19 2009 Liang Suilong <liangsuilong@gmail.com> 0.2.20090919-1
- Inital package for Fedora
