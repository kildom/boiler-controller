FROM ubuntu:22.04

ARG ARG_LINUX_PACKAGES=" \
    wget curl mc tzdata sudo git make gcc g++ \
"

ENV TZ=Etc/UTC

# Install linux packages (with cache)
RUN rm -f /etc/apt/apt.conf.d/docker-clean; echo 'Binary::apt::APT::Keep-Downloaded-Packages "true";' > /etc/apt/apt.conf.d/keep-cache
RUN --mount=type=cache,target=/var/cache/apt,sharing=locked \
	--mount=type=cache,target=/var/lib/apt,sharing=locked \
	apt update && \
	DEBIAN_FRONTEND=noninteractive apt install -y -qq $ARG_LINUX_PACKAGES

# Setup user
ARG UN
ARG UI
ARG GN
ARG GI

RUN groupadd -g $GI $GN && \
	useradd -s `which bash` -m -g $GI -G adm,dialout,cdrom,sudo,dip,plugdev -u $UI $UN && \
	echo "$UN ALL=(ALL:ALL) NOPASSWD: ALL" > /etc/sudoers.d/$UN
USER $UN:$GN
WORKDIR /home/$UN
ENV PATH=$PATH:/home/$UN/.local/bin
RUN mkdir -p /home/$UN/.local/bin && \
	mkdir -p /home/$UN/.cache

# Install Node.js 20
RUN curl -fsSL https://deb.nodesource.com/setup_20.x | sudo -E bash - && \
    sudo apt-get install -y nodejs

# Install WASI SDK
RUN wget https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-29/wasi-sdk-29.0-x86_64-linux.deb && \
    sudo dpkg -i wasi-sdk-29.0-x86_64-linux.deb && \
    rm wasi-sdk-29.0-x86_64-linux.deb


WORKDIR /home/$UN/app

