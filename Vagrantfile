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
end
