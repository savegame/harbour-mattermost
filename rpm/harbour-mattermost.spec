%define _aurora_os_build ru.sashikknox.wattermost

%if "0%{?_aurora_os_build}" != "0"
Name:  ru.sashikknox.wattermost
%define qmake_flags -r CONFIG+=auroraos CONFIG+=static_websocket
%else
Name:  harbour-mattermost
%define qmake_flags -r
%endif
# >> macros
%define __requires_exclude ^libQt5Core.so.5(Qt_5_PRIVATE_API)|libQt5Core.so.*|libQt5WebSockets.so.5(Qt_5)|libgcrypt*$
# % define __requires_exclude ^|libQt5WebSockets.so.*$
# << macros

%{!?qtc_qmake:%define qtc_qmake %qmake}
%{!?qtc_qmake5:%define qtc_qmake5 %qmake5}
%{!?qtc_make:%define qtc_make make}
%{?qtc_builddir:%define _builddir %qtc_builddir}

Summary:    Mattermost Sailfish client
Version:    0.1.10
Release:    7
Group:      Qt/Qt
License:    LICENSE
Source0:    %{name}-%{version}.tar.bz2
# harbour-mattermost - its old name same application 
# Obsoletes:  harbour-mattermost <= 0.1.7
Requires:   sailfishsilica-qt5 >= 0.10.9
BuildRequires:  pkgconfig(sailfishapp) >= 1.0.2
BuildRequires:  pkgconfig(sailfishsilica)
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  pkgconfig(Qt5DBus)
BuildRequires:  pkgconfig(Qt5Svg)
#BuildRequires:  pkgconfig(Qt5WebSockets)
BuildRequires:  pkgconfig(nemonotifications-qt5)
BuildRequires:  desktop-file-utils

%description
Mattermost native client for SailfishOS


%prep
%setup -q -n %{name}-%{version}

# >> setup
# << setup

%build
# >> build pre
# << build pre
#%qtc_qmake5 %qmake_flags harbour-mattermost.pro
qmake %qmake_flags harbour-mattermost.pro
%qtc_make
# >> build post
# << build post

%install
rm -rf %{buildroot}
# >> install pre
# << install pre
#%qmake5_install
make INSTALL_ROOT=%{buildroot} install
# >> install post
# << install post

#desktop-file-install --delete-original       \
#  --dir %{buildroot}%{_datadir}/applications             \
#   %{buildroot}%{_datadir}/applications/*.desktop

%files
%defattr(-,root,root,-)
%{_bindir}/%{name}
%{_datadir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.png
# >> files
# % {_datadir}/dbus-1/services
# << files
