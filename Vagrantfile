# -*- mode: ruby -*-
# vi: set ft=ruby :

# Vagrantfile API/syntax version. Don't touch unless you know what you're doing!
VAGRANTFILE_API_VERSION = "2"

Vagrant.configure(VAGRANTFILE_API_VERSION) do |config|
  vms = [
    {
      :id => "ubuntu-bionic",
      :box => "bento/ubuntu-18.04",
    },
    {
      :id => "ubuntu-focal",
      :box => "bento/ubuntu-20.04",
    },
    {
      :id => "ubuntu-hirsute",
      :box => "bento/ubuntu-21.04",
    },
    {
      :id => "ubuntu-impish",
      :box => "bento/ubuntu-21.10",
    },
    {
      :id => "debian-buster",
      :box => "bento/debian-10",
    },
    {
      :id => "debian-bullseye",
      :box => "bento/debian-11",
    },
    {
      :id => "almalinux-8",
      :box => "bento/almalinux-8",
    },
    {
      :id => "amazon-linux-2",
      :box => "bento/amazonlinux-2",
    },
    {
      :id => "centos-7",
      :box => "bento/centos-7",
    },
  ]

  n_cpus = ENV["VM_N_CPUS"] || 2
  n_cpus = Integer(n_cpus) if n_cpus
  memory = ENV["VM_MEMORY"] || 1024
  memory = Integer(memory) if memory
  vms.each do |vm|
    id = vm[:id]
    box = vm[:box] || id
    config.vm.define(id) do |node|
      node.vm.box = box
      node.vm.provider("virtualbox") do |virtual_box|
        virtual_box.cpus = n_cpus if n_cpus
        virtual_box.memory = memory if memory
      end
    end
  end
end
