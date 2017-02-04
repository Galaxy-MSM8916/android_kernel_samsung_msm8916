
ccflags-y ?=
ccflags-y += -O1 -fthread-jumps \
          -falign-functions -falign-jumps \
          -falign-loops -falign-labels \
          -fcrossjumping \
          -fcse-follow-jumps -fcse-skip-blocks \
          -fexpensive-optimizations \
          -fgcse -fgcse-lm \
          -fhoist-adjacent-loads \
          -finline-small-functions \
          -findirect-inlining \
          -fipa-cp \
          -fipa-sra \
          -fisolate-erroneous-paths-dereference \
          -foptimize-sibling-calls \
          -foptimize-strlen \
          -fpeephole2 \
          -frerun-cse-after-loop \
          -fstrict-aliasing \
          -ftree-switch-conversion -ftree-tail-merge \
          -ftree-pre \
          -ftree-vrp \
          -mtune=cortex-a57.cortex-a53