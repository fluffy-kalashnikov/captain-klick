some sort of handle representing a resource. the following descriptor types exist:

| type | shader visible, CPU write only | non shader visible, CPU read/write |
| ---- | ---- | ---- |
| [Constant Buffer View (CBV)](constant-buffer-view) | X | X |
| [Shader Resource View (SRV)](shader-resource-view) | X | X |
| [Unordered Access View (UAV)](unordered-access-view) |  | X |
| [Sampler](sampler) |  | X |
| [Index Buffer View (IBV)](index-buffer-view) |  | X |
| [Vertex Buffer View (VBV)](vertex-buffer-view) |  | X |
| [Stream Output Buffer View (SOV)](stream-output-buffer-view) |  | X |
| [Render Target View (RTV)](render-target-view) |  | X |
| [Depth Stencil View (DSV)](depth-stencil-view) |  | X |
