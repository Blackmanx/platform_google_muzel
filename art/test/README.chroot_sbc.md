# ART chroot-based testing on a Linux SBC

This doc describes how to set up a Linux SBC for running ART tests.

The SBC should support RVA22+V. However, some boards, such as Orange Pi RV2,
are almost compliant but get a SIGBUS/ADRALN for unaligned V memory access
instructions and we have a workaround for that deficiency, see below.

## Set up the SBC and development machine

The basic setup of the SBC is not covered by this document. We assume that the
SBC is already set up, running Linux (such as Ubuntu 24.04 LTS) and accepting
ssh connections. You need to export ART ssh connection details with
```
export ART_TEST_SSH_USER=orangepi
export ART_TEST_SSH_HOST=192.168.x.y
export ART_TEST_SSH_PORT=22
```
where the values depend on your local setup. In the example above, the host is
on a local network, `x` depends on the router setup and `y` is usually assigned
by DHCP and may change on reconnection.

ART test scripts are configured with `art/test/testrunner/ssh_config_sbc` for
SBC and use the SSH key `~/.ssh/sbc`. As a one-time setup for the development
machine and SBC, you need to generate that key and install it on the SBC to let
scripts connect without requiring a password.
```
ssh-keygen -f .ssh/sbc
ssh-copy-id -i ~/.ssh/sbc -p $ART_TEST_SSH_PORT -o IdentityAgent=none \
    -o StrictHostKeyChecking=no "$ART_TEST_SSH_USER@$ART_TEST_SSH_HOST"
```
You can test that the passwordless setup is working with
```
ssh -i ~/.ssh/sbc -p "$ART_TEST_SSH_PORT" "$ART_TEST_SSH_USER@$ART_TEST_SSH_HOST"
```

ART chroot tests on SBC require `sudo mount` and `sudo umount`. To let ART
test scripts issue these commands, please edit the `sudoers` file on the SBC.
For simplicity, you can use `NOPASSWD:ALL`. See `man sudoers` for details.

# Run ART tests

This is done in the same way as you would run tests in chroot on device (except
for a few extra environment variables). In addition to `ART_TEST_SSH_*`, use
```
export ART_TEST_ON_SBC=true
```
and if you need a workaround for unaligned V memory access, also
```
export ART_TEST_ON_SBC_RISCV64_V_ADRALN_WORKAROUND=true
```
Note: You should `unset ART_TEST_CHROOT` if you have previously exported it.
You should rely on the default values instead. Some scripts allow the variable
to override the default values while other scripts don't, so non-standard
values may not work. (TODO: Do not hardcode `ART_TEST_CHROOT` for
`ART_TEST_ON_{VM,SBC}` in `buildbot-utils.h`.)

Then you can use the standard build and test steps.
```
. ./build/envsetup.sh
lunch aosp_riscv64-trunk_staging-userdebug  # or armv8-trunk_staging-eng, etc.
art/tools/buildbot-build.sh --target  # --installclean

art/tools/buildbot-cleanup-device.sh

# The following two steps can be skipped for faster iteration, but it doesn't
# always track and update dependencies correctly (e.g. if only an assembly file
# has been modified).
art/tools/buildbot-setup-device.sh
art/tools/buildbot-sync.sh

# Use `/home/ART_TEST_SSH_USER/art-test-chroot` instead of `$ART_TEST_CHROOT.`
art/test/run-test --chroot /home/$ART_TEST_SSH_USER/art-test-chroot \
    --64 --interpreter -O 001-HelloWorld
art/test.py --target -r --ndebug --no-image --64 --interpreter  # specify tests
art/tools/run-gtests.sh

art/tools/buildbot-cleanup-device.sh
```
Both `test.py` and `run-test` scripts can be used. Tweak options as necessary.

# Limitations

There are some limitations on Linux SBCs, similar to running tests on the host
or a Linux VM. At the time of writing, only one gtest was disabled using the
`TEST_DISABLED_ON_SBC` macro and one run-test disabled in `knownfailures.json`.
