%define _unpackaged_files_terminate_build 0
Name:		wifi-efl-ug
Summary:	Wi-Fi UI Gadget for TIZEN
Version:	1.0.170
Release:	1
Group:		App/Network
License:	Flora-1.1
Source0:	%{name}-%{version}.tar.gz

BuildRequires:	pkgconfig(ecore)
BuildRequires:	pkgconfig(ecore-imf)
BuildRequires:	pkgconfig(ecore-input)
BuildRequires:	pkgconfig(appcore-efl)
BuildRequires:	pkgconfig(elementary)
BuildRequires:	pkgconfig(efl-assist)
BuildRequires:	pkgconfig(glib-2.0)
BuildRequires:	pkgconfig(openssl)
BuildRequires:	pkgconfig(cert-svc-vcore)
BuildRequires:	pkgconfig(ui-gadget-1)
BuildRequires:	pkgconfig(sensor)
BuildRequires:	pkgconfig(capi-network-wifi)
BuildRequires:	pkgconfig(capi-network-connection)
BuildRequires:	pkgconfig(capi-network-tethering)
BuildRequires:	pkgconfig(capi-ui-efl-util)
BuildRequires:	pkgconfig(network)
BuildRequires:	pkgconfig(feedback)
BuildRequires:	pkgconfig(efl-extension)
BuildRequires:	pkgconfig(aul)
#BuildRequires:  pkgconfig(setting-common-internal)
#BuildRequires:  pkgconfig(setting-lite-common-internal)
BuildRequires:	cmake
BuildRequires:	gettext-tools
BuildRequires:	edje-tools
Requires(post):		/sbin/ldconfig
requires(postun):	/sbin/ldconfig

%description
Wi-Fi UI Gadget

%if "%{profile}" == "mobile"
%package -n net.wifi-qs
Summary:    Wi-Fi System popup
Requires:   %{name} = %{version}

%description -n net.wifi-qs
Wi-Fi System popup for TIZEN
%endif

%if "%{profile}" == "wearable"
%package -n org.tizen.w-wifi
Summary:    Wi-Fi UI Gadget for wearable

%description -n org.tizen.w-wifi
Wi-Fi UI Gadget for wearable
%endif

%prep
%setup -q

%define PREFIX /usr/

%build
#LDFLAGS+="-Wl,--rpath=%{PREFIX}/lib -Wl,--as-needed"
cmake -DCMAKE_INSTALL_PREFIX=%{PREFIX} \
%if ! 0%{?model_build_feature_network_tethering_disable}
	-DTIZEN_TETHERING_ENABLE=1 \
%endif
	-DMODEL_BUILD_FEATURE_WLAN_CONCURRENT_MODE=1 \
%if "%{profile}" == "mobile"
	-DTIZEN_MOBILE=1 \
%endif
	.

make %{?_smp_mflags}


%install
%make_install

#License
mkdir -p %{buildroot}%{_datadir}/license
cp LICENSE %{buildroot}%{_datadir}/license/wifi-efl-ug
%if "%{profile}" == "mobile"
cp LICENSE %{buildroot}%{_datadir}/license/net.wifi-qs
%endif
%if "%{profile}" == "wearable"
cp LICENSE %{buildroot}%{_datadir}/license/org.tizen.w-wifi
%endif

%post
/sbin/ldconfig

mkdir -p %{PREFIX}/bin/
mkdir -p /usr/apps/wifi-efl-ug/bin/ -m 777

%postun -p /sbin/ldconfig

%files
%manifest wifi-efl-ug.manifest
%{PREFIX}/ug/lib/*
%attr(644,-,-) %{PREFIX}/ug/lib/*
%attr(755,-,-) %{PREFIX}/ug/lib/
%{PREFIX}/apps/wifi-efl-ug/res/edje/wifi-efl-UG/*.edj
%{_datadir}/license/wifi-efl-ug
%{_datadir}/packages/wifi-efl-ug.xml
%if "%{profile}" == "mobile"
%{_datadir}/locale/*/LC_MESSAGES/*.mo
%{_datadir}/icons/*.png
%endif
/usr/apps/wifi-efl-ug/shared/res/tables/ug-wifi-efl_ChangeableColorTable.xml
/usr/apps/wifi-efl-ug/shared/res/tables/ug-wifi-efl_FontInfoTable.xml

%if "%{profile}" == "mobile"
%files -n net.wifi-qs
%manifest net.wifi-qs.manifest
%{_bindir}/wifi-qs
%{_datadir}/packages/net.wifi-qs.xml
%{_datadir}/icons/*.png
%{PREFIX}/apps/wifi-efl-ug/res/edje/wifi-qs/*.edj
%{_datadir}/license/net.wifi-qs
%endif

%if "%{profile}" == "wearable"
%files -n org.tizen.w-wifi
%manifest org.tizen.w-wifi.manifest
/usr/shared/res/tables/color_table.xml
/usr/shared/res/tables/font_table.xml
%defattr(-,root,root,-)
%attr(-,inhouse,inhouse)
%{PREFIX}/apps/org.tizen.w-wifi/bin/*
#%{PREFIX}/res/images/*.png
%{PREFIX}/apps/org.tizen.w-wifi/res/locale/*/LC_MESSAGES/*.mo
%{PREFIX}/apps/org.tizen.w-wifi/res/edje/*
%{_datadir}/packages/org.tizen.w-wifi.xml
%{_datadir}/license/org.tizen.w-wifi
%endif
