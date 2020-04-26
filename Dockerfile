FROM archlinux

RUN pacman -Sy --noconfirm
RUN pacman -S gcc bazel git valgrind iproute2 --noconfirm

WORKDIR /eventgateway
COPY . .
RUN bazel build //main:all --curses=no --color=no --noshow_progress

ENV SERVER_PORT=${server_port}

ENTRYPOINT [ "./run.sh" ]
