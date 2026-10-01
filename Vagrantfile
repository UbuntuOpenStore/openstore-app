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
    apt-get install -y software-properties-common
    add-apt-repository -y ppa:lomiri/builds
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
      qml6-module-lomiri-connectivity

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
    nmcli connection up openstore-online

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
      exit 1
    elif [ -z "$state" ]; then
      echo "Could not read PackageKit's NetworkState, skipping the online check." >&2
    else
      echo "PackageKit reports the VM as online (NetworkState=$state)."
    fi
  SHELL
end
