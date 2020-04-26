#include "DisplayList.h"

void DisplayList::process(Gfx *input) {
  while(input->type != PCG_END_DISPLAY_LIST) {
    if(input->type == PCG_BRANCH_LIST) {
      input = (Gfx*)input->data.display_list;
    } else if(input->type == PCG_DISPLAY_LIST) {
      process((Gfx*)input->data.display_list);
      input++;
    } else {
      insert(*input);
      input++;
    }
  }
}

void DisplayList::insert(const Gfx &data) {
  if(used_nodes == node_count) {
    printf("DisplayList overflow at %d nodes!\n", used_nodes);
    assert(false);
  }

  nodes[used_nodes] = data;

  switch(data.type) {
    case PCG_VIEWPORT:
    {
      auto vp = pool.alloc<Vp>();
      *vp = *data.data.viewport.ptr;
      nodes[used_nodes].data.viewport.ptr = vp;
    } break;

    case PCG_VERTEX:
    {
      auto v = pool.alloc<Vtx>(data.data.vertex.n);
      for(int i = 0; i < data.data.vertex.n; i++) {
        v[i] = data.data.vertex.ptr[i];
      }
      nodes[used_nodes].data.vertex.ptr = v;
    } break;

    // TODO TEXTURE PCG_SET_TEX_IM

    // TODO COLOR IMAGE PCG_SET_COLOR_IMAGE

    case PCG_MATRIX:
    {
      auto m = pool.alloc<Mtx>();
      *m = *(Mtx*)data.data.matrix.ptr;
      nodes[used_nodes].data.matrix.ptr = m;
    } break;

    // TODO TEX BLOCK PCG_LOAD_TEX_BLOCK

    // TODO LIGHT PCG_LIGHT
  }

  used_nodes++;
}