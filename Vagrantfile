Vagrant.configure("2") do |config|
  config.vm.box = "cloud-image/ubuntu-26.04"
  config.vm.hostname = "openstore-build"

  config.vm.synced_folder ".", "/vagrant", disabled: true

  config.vm.provider "qemu" do |qe|
    qe.memory = 16384
    qe.cpus = 8
    qe.other_default = %W(-parallel none -monitor none)
    qe.extra_qemu_args = %W(-device virtio-vga-gl,edid=on -display gtk,gl=on)
    config.vm.synced_folder ".", "/home/vagrant/openstore-app", type: "sshfs"
  end

  config.vm.provision "shell", inline: <<-SHELL
    set -euxo pipefail
    export DEBIAN_FRONTEND=noninteractive

    apt-get update
    apt-get install -y software-properties-common curl
    add-apt-repository -y ppa:lomiri/builds

    # Use the Ubport repo for packages not available in the lomiri ppa
    install -d -m 0755 /etc/apt/keyrings
    curl -fsSL https://repo.ubports.com/keyring.gpg -o /etc/apt/keyrings/ubports.gpg
    cat > /etc/apt/sources.list.d/ubports.sources <<'EOF'
Types: deb
URIs: https://repo.ubports.com
Suites: 26.04-1.x
Components: main
Signed-By: /etc/apt/keyrings/ubports.gpg
EOF
    cat > /etc/apt/preferences.d/ubports.pref <<'EOF'
Package: *
Pin: origin repo.ubports.com
Pin-Priority: 100

Package: ubuntutouch-artwork qml-module-snapd2
Pin: origin repo.ubports.com
Pin-Priority: 501
EOF

    apt-get update

    apt-get install -y \
      lomiri-desktop \
      lightdm \
      cmake \
      build-essential \
      libclick-dev \
      libpam0g-dev \
      libsnapd-qt-dev \
      libsnapd-glib-dev \
      qt6-base-dev \
      qt6-declarative-dev \
      qt6-webengine-dev \
      libpackagekitqt6-dev \
      libappstreamqt-dev \
      libxapian-dev \
      intltool \
      qml6-module-lomiri-content \
      qml6-module-lomiri-connectivity \
      morph-browser-qt6 \
      apt-config-icons \
      apt-config-icons-hidpi \
      apt-config-icons-large \
      apt-config-icons-large-hidpi

    usermod -s /bin/bash vagrant
    usermod -aG sudo vagrant

    # Dev bypass: let the vagrant user install/remove packages without a polkit prompt.
    install -d /etc/polkit-1/rules.d
    cat > /etc/polkit-1/rules.d/49-openstore-dev.rules <<'EOF'
polkit.addRule(function(action, subject) {
    if (subject.user == "vagrant" &&
        (action.id == "org.freedesktop.packagekit.package-install" ||
         action.id == "org.freedesktop.packagekit.package-install-untrusted" ||
         action.id == "org.freedesktop.packagekit.package-remove"))
        return polkit.Result.YES;
});
EOF
    systemctl restart polkit 2>/dev/null || systemctl restart polkitd 2>/dev/null || true

    # Make lightdm the display manager and boot into the graphical target
    echo 'lightdm shared/default-x-display-manager select lightdm' | debconf-set-selections
    dpkg-reconfigure -f noninteractive lightdm || true
    systemctl set-default graphical.target
    systemctl enable lightdm

    # Use the Lomiri desktop session and auto-login as vagrant
    mkdir -p /etc/lightdm/lightdm.conf.d
    cat > /etc/lightdm/lightdm.conf.d/50-lomiri.conf <<'EOF'
[Seat:*]
user-session=lomiri
autologin-user=vagrant
autologin-session=lomiri
EOF
  SHELL

  # PackageKit sees the VM as offline because NetworkManager manages no interfaces,
  # so give it a dummy interface with a static address and a default route (route
  # metric kept worse than the real uplink's so it never takes precedence).
  # Re-applied on every boot so it survives restarts.
  config.vm.provision "packagekit-online", type: "shell", run: "always", inline: <<-SHELL
    set -euxo pipefail

    systemctl enable --now NetworkManager

    for _ in $(seq 1 30); do
      nmcli general status >/dev/null 2>&1 && break
      sleep 1
    done

    # Back the interface with the dummy kernel module.
    modprobe dummy || true
    for _ in $(seq 1 15); do
      nmcli --terse --fields DEVICE device status | grep -qx dummy0 && break
      sleep 1
    done

    if ! nmcli connection show openstore-online >/dev/null 2>&1; then
      nmcli connection add \
        type dummy \
        ifname dummy0 \
        con-name openstore-online \
        connection.autoconnect yes \
        ipv4.method manual \
        ipv4.addresses 192.0.2.1/24 \
        ipv4.gateway 192.0.2.2 \
        ipv4.route-metric 4200 \
        ipv6.method disabled
    fi

    nmcli device set dummy0 managed yes 2>/dev/null || true
    sleep 2
    nmcli connection up openstore-online 2>/dev/null || nmcli connection up openstore-online || true

    nmcli device status

    # NetworkManager state changes are picked up by GNetworkMonitor, which is
    # what PackageKit asks for its NetworkState (0 unknown, 1 offline, 2+ online).
    systemctl try-restart packagekit || true
    state=""
    for _ in $(seq 1 15); do
      state=$(busctl get-property org.freedesktop.PackageKit /org/freedesktop/PackageKit \
        org.freedesktop.PackageKit NetworkState 2>/dev/null | awk '{print $2}' || true)
      case "$state" in
        2|3|4|5) break ;;
      esac
      sleep 1
    done

    if [ "$state" = "0" ] || [ "$state" = "1" ]; then
      echo "PackageKit still reports the VM as offline (NetworkState=$state)." >&2
      echo "Installs will fail with 'Cannot download packages whilst offline'." >&2
      if nmcli connection show --active | grep -q openstore-online && ping -c 1 -W 2 8.8.8.8 >/dev/null 2>&1; then
        echo "Continuing despite PackageKit reporting offline (network is up)." >&2
      else
        exit 1
      fi
    elif [ -z "$state" ]; then
      echo "Could not read PackageKit's NetworkState, skipping the online check." >&2
    else
      echo "PackageKit reports the VM as online (NetworkState=$state)."
    fi
  SHELL
end
