FROM ubuntu:18.04
WORKDIR /root
RUN apt-get update && apt-get install -y \
  bzip2 \
  curl \
  doxygen \
  g++ \
  gcc \
  git \
  libcrypto++-dev \
  libgmp3-dev \
  m4 \
  make \
  patch \
  python \
  python3 \
  tmux \
  unzip \
  vim \
  wget \
  yasm

ADD source/ /root/source
ADD config/ /root/config
ADD depends.sh .
RUN ["bash", "depends.sh"]

ADD install.sh .
RUN ["bash", "install.sh"]

ADD Cert-Store /root/SCALE-MAMBA/Cert-Store
ADD inputs/decrypt_input.txt /root/SCALE-MAMBA
ADD ec2_benchmark.sh /root/SCALE-MAMBA
ADD dec_input_gen.py /root/SCALE-MAMBA

CMD ["/bin/bash"]
