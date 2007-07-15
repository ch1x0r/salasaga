Name: flame_project
Summary: An Integrated Development Environment for producing Animated SVG files.
Version: 0.7.7.dev
Release: 6
License: LGPL
Group: Applications/Multimedia
URL: http://www.flameproject.org/

Source0: http://downloads.sourceforge.net/flameproject/flame_project-%{version}.tar.bz2
BuildRoot: %{_tmppath}/%{name}-root

Requires: glib2, gtk2, libgnome, libxml2
BuildRequires: autoconf, rpm-devel >= 4.2.1, make, gcc, glib2-devel, gtk2-devel, libgnome-devel, libxml2-devel

%description
An Integrated Development Environment for producing Animated SVG files.

%prep
%setup
autoconf

%build
%configure
make

%install

#rm -fr %{buildroot}

%makeinstall

%clean
#rm -fr %{buildroot}

%files 
%defattr(-,root,root) 
%{_bindir}/* 
%{_datadir}/* 

%changelog
* Sun Jul 15 2007 Justin Clift <justin@postgresql.org> 0.7.7-dev-6
- Added initial Flame icon suitable for gnome menu bar
- Shortcut now added to gnome menu

* Tue Jul 09 2007 Justin Clift <justin@postgresql.org> 0.7.7-dev-5
- No longer requires 1280x1024 resolution or higher.  Even 800x600 works.
- Refactored the film strip code to use a GtkListStore
  - Selected slide is now obvious
  - Added "move to top" and "move to bottom" options
  - Remapped the "Delete Slide" key to use Alt-i-d, easier to remember
  - Changing width of film strip is fairly smooth and resizes thumbnails
  - Changed film strip width is saved between sessions
  - Changed film strip width also gets updated into the Preferences
- Path to icon and sound file(s) are now calculated dynamically
- Bug fixes with exported SVG code.  Multi-line text works ok now.

* Wed Jun 27 2007 Justin Clift <justin@postgresql.org> 0.7.7-dev-4
- Bug fix version, removing known crashes.
- Screenshot slides now deleted after import.

* Fri Jan 05 2007 Justin Clift <justin@postgresql.org> 0.7.7-dev-2
- Next version of the package, with slightly improved SVG output.

* Fri Jan 05 2007 Justin Clift <justin@postgresql.org> 0.7.7-dev-1
- Initial package.
