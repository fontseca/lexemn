#! /bin/bash

update() {
  sudo apt update
}

install_dependencies() {
  # Install GNU Readline for development
  sudo apt install libreadline-dev
}

update
install_dependencies
