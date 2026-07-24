FROM debian:bookworm-slim AS thaios-builder

LABEL description="ThaiOS ISO Builder"
LABEL version="1.0"

# Install build dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    debootstrap \
    squashfs-tools \
    xorriso \
    grub-pc-bin \
    grub-efi-amd64-bin \
    grub-common \
    systemd-container \
    ca-certificates \
    librsvg2-bin \
    curl \
    git \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build

# Copy ThaiOS source
COPY . .

# Build the ISO
RUN bash scripts/build-thaios-iso.sh all

# Final stage: just the ISO
FROM scratch AS thaios-iso
COPY --from=thaios-builder /build/build/ThaiOS-1.0.iso /
