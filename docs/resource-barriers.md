# Rules
1. CONSTRAIN STATES. Avoid COMMON and GENERIC_READ states. GENERIC_READ can be used for upload heaps, transitions with GENERIC_READ is a no-no. COMMON is needed for CPU access to textures and copy engine. Do not create both a pixel shader resource view and non-pixel shader resource view if not needed.
2. AVOID TRANSITIONS.
3. BATCH BARRIERS.
4. [Split barriers](split-barriers) can be used for driver optimizations in very specific cases