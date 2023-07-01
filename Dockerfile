FROM mambaorg/micromamba:1.4.5
COPY --chown=$MAMBA_USER:$MAMBA_USER environment.yml /tmp/env.yaml
COPY --chown=$MAMBA_USER:$MAMBA_USER app /app
COPY --chown=$MAMBA_USER:$MAMBA_USER src /src
RUN micromamba install -y -n base -f /tmp/env.yaml && micromamba clean --all --yes
WORKDIR /
RUN make
ENTRYPOINT ["/usr/local/bin/_entrypoint.sh", "panel", "serve", "app", "--allow-websocket-origin=*"]