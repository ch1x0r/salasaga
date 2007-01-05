Name: flame_project
Summary: GUI eLearning creation tool for applications, with output in animated SVG.
Version: 0.7.7.dev
Release: 1
License: LGPL
Group: Applications/Multimedia
URL: http://www.flameproject.org/

Source0: http://downloads.sourceforge.net/flameproject/flame_project-%{version}.tar.bz2
BuildRoot: %{_tmppath}/%{name}-root

Requires: glib2, gtk2, libgnome, libxml2
BuildRequires: autoconf, rpm-devel >= 4.2.1, make, gcc, glib2-devel, gtk2-devel, libgnome-devel, libxml2-devel

%description
GUI eLearning creation tool for applications, with output in animated SVG.

%prep
%setup
autoconf

%{__cat} <<EOF >flameproject.desktop
[Desktop Entry]
Name=The Flame Project
Comment=GUI eLearning creation tool for applications, with output in animated SVG.
Icon=flame_icon.ico
Exec=flame-edit
Type=Application
Terminal=false
StartupNotify=true
Categories=Graphics;2DGraphics;RasterGraphics;GTK;
Encoding=UTF-8
EOF

%build
%configure
make

%install

%if %{?_without_freedesktop:1}0
	%{__install} -D -m0644 smart-gui.desktop %{buildroot}%{_datadir}/gnome/apps/Graphics/flameproject.desktop
%else
	%{__install} -d -m0755 %{buildroot}%{_datadir}/applications/
	desktop-file-install --vendor "" \
		--dir %{buildroot}%{_datadir}/applications \
		--add-category X-Red-Hat-Extra \
		flameproject.desktop
%endif

#rm -fr %{buildroot}

%makeinstall

%clean
#rm -fr %{buildroot}

%files 
%defattr(-,root,root) 
%{_bindir}/* 
%{_datadir}/* 

%changelog
* Fri Jan 05 2007 Justin Clift <justin@postgresql.org> 0.7.7-dev-1
- Initial package.
