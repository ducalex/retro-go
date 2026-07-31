#ifndef H_PHD
#define H_PHD

#include "common.h"
#include "stream.h"

struct PHD_Room {
    int32 x, z;
    int32 yBottom, yTop;
    uint32 dataSize;
    uint16 numPortals;
    uint16 numVertices;
    uint16 numQuads;
    uint16 numTriangles;
    uint16 numSprites;
    uint16 numLights;
    uint16 numMeshes;
    int16  ambient;
    uint8  numSectorsX, numSectorsZ;
};

bool read_PHD(DataStream &f)
{
    if (!f.data) return false;

    uint8* ptr = gLevelData;
    rg_system_log(RG_LOG_INFO, "OpenLara", "read_PHD: START gLevelData=%p\n", ptr);

    uint32 magic = f.read32u();
    if (magic != 0x00000020) {
        rg_system_log(RG_LOG_ERROR, "OpenLara", "read_PHD: invalid magic %08x\n", (unsigned int)magic);
        return false;
    }

    level.version = VER_TR1_PC;
    
    level.tilesCount = f.read32u();
    level.tiles = (const uint8*)f.getPtr();
    f.seek(level.tilesCount * 256 * 256 + 4);

    level.roomsCount = f.read16u();
    ptr = (uint8*)(((intptr_t)ptr + 3) & ~3);
    level.roomsInfo = (const RoomInfo*)ptr;
    ptr += level.roomsCount * sizeof(RoomInfo);

    for (uint32 i = 0; i < level.roomsCount; i++)
    {
        PHD_Room r;
        r.x = f.read32s(); r.z = f.read32s(); r.yBottom = f.read32s(); r.yTop = f.read32s();
        r.dataSize = f.read32u();
        r.numPortals = f.read16u(); r.numVertices = f.read16u();
        r.numQuads = f.read16u(); r.numTriangles = f.read16u();
        r.numSprites = f.read16u(); r.numLights = f.read16u();
        r.numMeshes = f.read16u(); r.ambient = f.read16s();
        r.numSectorsZ = f.read8u(); r.numSectorsX = f.read8u();

        RoomInfo* info = (RoomInfo*)level.roomsInfo + i;
        info->x = r.x >> 8; info->z = r.z >> 8; info->yBottom = r.yBottom; info->yTop = r.yTop;
        info->verticesCount = r.numVertices; info->quadsCount = r.numQuads; info->trianglesCount = r.numTriangles;
        info->spritesCount = r.numSprites; info->portalsCount = (uint8)r.numPortals;
        info->lightsCount = (uint8)r.numLights; info->meshesCount = (uint8)r.numMeshes;
        info->ambient = (uint8)(r.ambient >> 8); info->xSectors = r.numSectorsX; info->zSectors = r.numSectorsZ;

        ptr = (uint8*)(((intptr_t)ptr + 3) & ~3);
        info->data.vertices = (const RoomVertex*)ptr;
        for (uint32 j = 0; j < info->verticesCount; j++) {
            RoomVertex *v = (RoomVertex*)ptr; ptr += sizeof(RoomVertex);
            v->x = f.read16s(); v->y = f.read16s(); v->z = f.read16s(); v->g = f.read16u() >> 8;
        }

        ptr = (uint8*)(((intptr_t)ptr + 3) & ~3);
        info->data.quads = (const RoomQuad*)ptr;
        for (uint32 j = 0; j < info->quadsCount; j++) {
            RoomQuad *q = (RoomQuad*)ptr; ptr += sizeof(RoomQuad);
            for (int k=0; k<4; k++) q->indices[k] = f.read16u(); q->flags = f.read16u();
        }

        ptr = (uint8*)(((intptr_t)ptr + 3) & ~3);
        info->data.triangles = (const RoomTriangle*)ptr;
        for (uint32 j = 0; j < info->trianglesCount; j++) {
            RoomTriangle *t = (RoomTriangle*)ptr; ptr += sizeof(RoomTriangle);
            for (int k=0; k<3; k++) t->indices[k] = f.read16u(); t->flags = f.read16u();
        }

        ptr = (uint8*)(((intptr_t)ptr + 3) & ~3);
        info->data.sprites = (const RoomSprite*)ptr;
        for (uint32 j = 0; j < info->spritesCount; j++) {
            RoomSprite *s = (RoomSprite*)ptr; ptr += sizeof(RoomSprite);
            int16 vertex = f.read16s(); s->index = (uint8)f.read16u();
            s->pos = _vec3s(0,0,0); s->g = 0; // TR1 room sprites are linked to vertices
        }

        ptr = (uint8*)(((intptr_t)ptr + 3) & ~3);
        info->data.portals = (const Portal*)ptr;
        for (uint32 j = 0; j < info->portalsCount; j++) {
            Portal *p = (Portal*)ptr; ptr += sizeof(Portal);
            p->roomIndex = f.read16u(); f.read16s(); f.read16s(); f.read16s(); // normal
            for (int k=0; k<4; k++) { p->v[k].x = f.read16s(); p->v[k].y = f.read16s(); p->v[k].z = f.read16s(); }
        }

        ptr = (uint8*)(((intptr_t)ptr + 3) & ~3);
        info->data.sectors = (const Sector*)ptr;
        for (uint32 j = 0; j < uint32(info->zSectors) * info->xSectors; j++) {
            Sector *s = (Sector*)ptr; ptr += sizeof(Sector);
            s->floorIndex = f.read16u(); s->boxIndex = f.read16u(); s->roomBelow = f.read8u(); s->floor = f.read8s(); s->roomAbove = f.read8u(); s->ceiling = f.read8s();
        }

        ptr = (uint8*)(((intptr_t)ptr + 3) & ~3);
        info->data.lights = (const Light*)ptr;
        for (uint32 j = 0; j < info->lightsCount; j++) {
            Light *l = (Light*)ptr; ptr += sizeof(Light);
            int32 lx = f.read32s(), ly = f.read32s(), lz = f.read32s();
            l->pos = _vec3s((lx - r.x) >> 8, ly >> 8, (lz - r.z) >> 8);
            l->intensity = (uint8)(f.read16u() >> 8); l->radius = (uint8)(f.read32u() >> 10);
        }

        ptr = (uint8*)(((intptr_t)ptr + 3) & ~3);
        info->data.meshes = (const RoomMesh*)ptr;
        for (uint32 j = 0; j < info->meshesCount; j++) {
            RoomMesh *m = (RoomMesh*)ptr; ptr += sizeof(RoomMesh);
            int32 mx = f.read32s(), my = f.read32s(), mz = f.read32s();
            int16 angle = f.read16s(); uint16 flags = f.read16u();
            m->xy = ((mx - r.x) << 16) | (uint16)my;
            m->zf = ((mz - r.z) << 16) | flags;
        }

        info->alternateRoom = (uint8)f.read16s();
        info->flags = (uint8)f.read16u();

        Room* room = rooms + i; room->info = info; room->data = info->data; room->sectors = info->data.sectors; room->firstItem = NULL;
    }

    { // floors data
        uint32 floorsCount = f.read32u();
        ptr = (uint8*)(((intptr_t)ptr + 3) & ~3);
        level.floors = (const FloorData*)ptr;
        for (uint32 i = 0; i < floorsCount; i++) ((FloorData*)level.floors)[i] = f.read16u();
        ptr += floorsCount * 2;
    }

    { // mesh data
        uint32 meshDataCount = f.read32u(); int32 meshDataPos = f.getPos(); f.seek(meshDataCount * 2);
        level.meshesCount = f.read32u();
        ptr = (uint8*)(((intptr_t)ptr + 3) & ~3);
        int32* meshOffsets = (int32*)ptr; level.meshOffsets = meshOffsets;
        ptr += level.meshesCount * 4;
        for (uint32 i = 0; i < level.meshesCount; i++) meshOffsets[i] = f.read32s();

        int32 endPos = f.getPos();
        ptr = (uint8*)(((intptr_t)ptr + 3) & ~3);
        const Mesh** meshes_ptr = (const Mesh**)ptr; level.meshes = meshes_ptr;
        ptr += level.meshesCount * 4;

        for (int32 i = 0; i < (int)level.meshesCount; i++) {
            if (meshOffsets[i] < 0) { meshes_ptr[i] = NULL; continue; }
            bool found = false;
            for (int j = 0; j < i; j++) if (level.meshOffsets[j] == meshOffsets[i]) { meshes_ptr[i] = meshes_ptr[j]; found = true; break; }
            if (found) continue;

            f.setPos(meshDataPos + meshOffsets[i]);
            ptr = (uint8*)(((intptr_t)ptr + 3) & ~3);
            Mesh* mesh = (Mesh*)ptr; meshes_ptr[i] = mesh; ptr += sizeof(Mesh);
            mesh->center.x = f.read16s(); mesh->center.y = f.read16s(); mesh->center.z = f.read16s();
            mesh->radius = f.read16s(); mesh->intensity = 4096;
            uint16 mFlags = f.read16u();
            mesh->vCount = (uint8)X_MIN(255, f.read16u());
            int32 vPos = f.getPos(); f.seek(mesh->vCount * 6);
            mesh->hasNormals = (mFlags & 1);
            f.seek(mesh->hasNormals ? mesh->vCount * 6 : mesh->vCount * 2);
            
            mesh->rCount = f.read16s();
            MeshQuad* quads = (MeshQuad*)ptr; ptr += mesh->rCount * sizeof(MeshQuad);
            for (int j=0; j<mesh->rCount; j++) { for(int k=0;k<4;k++) quads[j].indices[k] = (uint8)f.read16u(); quads[j].flags = f.read16u(); }
            
            mesh->tCount = f.read16s();
            MeshTriangle* triangles = (MeshTriangle*)ptr; ptr += mesh->tCount * sizeof(MeshTriangle);
            for (int j=0; j<mesh->tCount; j++) { for(int k=0;k<3;k++) triangles[j].indices[k] = (uint8)f.read16u(); triangles[j].indices[3]=0; triangles[j].flags = f.read16u(); }
            
            MeshVertex* vertices = (MeshVertex*)ptr; ptr += mesh->vCount * sizeof(MeshVertex);
            int32 nextPos = f.getPos(); f.setPos(vPos);
            for (int j=0; j<mesh->vCount; j++) { vertices[j].x = f.read16s(); vertices[j].y = f.read16s(); vertices[j].z = f.read16s(); }
            f.setPos(nextPos);
        }
        f.setPos(endPos);
    }

    { // anims
        uint32 animsCount = f.read32u(); ptr = (uint8*)(((intptr_t)ptr + 3) & ~3);
        level.anims = (const Anim*)ptr;
        for (uint32 i = 0; i < animsCount; i++) {
            Anim* anim = (Anim*)ptr; ptr += sizeof(Anim);
            anim->frameOffset = f.read32u(); anim->frameRate = f.read8u(); anim->frameSize = f.read8u(); anim->state = f.read16u();
            anim->speed = f.read32u(); anim->accel = f.read32u(); anim->frameBegin = f.read16u(); anim->frameEnd = f.read16u();
            anim->nextAnimIndex = f.read16u(); anim->nextFrameIndex = f.read16u(); anim->statesCount = f.read16u(); anim->statesStart = f.read16u();
            anim->commandsCount = f.read16u(); anim->commandsStart = f.read16u();
        }
    }

    { // anim states
        uint32 animStatesCount = f.read32u(); ptr = (uint8*)(((intptr_t)ptr + 3) & ~3);
        level.animStates = (const AnimState*)ptr;
        for (uint32 i = 0; i < animStatesCount; i++) {
            AnimState* s = (AnimState*)ptr; ptr += sizeof(AnimState);
            s->state = uint8(f.read16u()); s->rangesCount = uint8(f.read16u()); s->rangesStart = f.read16u();
        }
    }

    { // anim ranges
        uint32 animRangesCount = f.read32u(); ptr = (uint8*)(((intptr_t)ptr + 3) & ~3);
        level.animRanges = (const AnimRange*)ptr;
        for (uint32 i = 0; i < animRangesCount; i++) {
            AnimRange* r = (AnimRange*)ptr; ptr += sizeof(AnimRange);
            r->frameBegin = f.read16u(); r->frameEnd = f.read16u(); r->nextAnimIndex = f.read16u(); r->nextFrameIndex = f.read16u();
        }
    }

    { // anim commands
        uint32 animCommandsCount = f.read32u(); ptr = (uint8*)(((intptr_t)ptr + 3) & ~3);
        level.animCommands = (const int16*)ptr;
        for (uint32 i = 0; i < animCommandsCount; i++) ((int16*)ptr)[i] = f.read16s();
        ptr += animCommandsCount * 2;
    }

    { // nodes
        uint32 nodesSize = f.read32u(); ptr = (uint8*)(((intptr_t)ptr + 3) & ~3);
        level.nodes = (const ModelNode*)ptr;
        for (uint32 i = 0; i < nodesSize / 16; i++) {
            ModelNode* n = (ModelNode*)ptr; ptr += sizeof(ModelNode);
            n->flags = uint16(f.read32u()); n->pos.x = int16(f.read32s()); n->pos.y = int16(f.read32s()); n->pos.z = int16(f.read32s());
        }
    }

    { // anim frames
        uint32 animFramesCount = f.read32u(); ptr = (uint8*)(((intptr_t)ptr + 3) & ~3);
        level.animFrames = (const uint16*)ptr;
        for (uint32 i = 0; i < animFramesCount; i++) ((uint16*)ptr)[i] = f.read16u();
        ptr += animFramesCount * 2;
    }

    { // models
        level.modelsCount = f.read32u(); ptr = (uint8*)(((intptr_t)ptr + 3) & ~3);
        Model* m_base = (Model*)ptr; level.models = m_base;
        ptr += MAX_MODELS * sizeof(Model);
        memset(m_base, 0, MAX_MODELS * sizeof(Model));
        for (uint32 i = 0; i < level.modelsCount; i++) {
            uint32 type = f.read32u();
            Model m;
            m.type = (uint16)type; m.count = (int16)f.read16u(); m.start = f.read16u();
            m.nodeIndex = f.read32u() / 16; f.seek(4); m.animIndex = f.read16u();
            if (type < MAX_MODELS) m_base[type] = m;
        }
    }

    { // static meshes
        level.staticMeshesCount = f.read32u(); ptr = (uint8*)(((intptr_t)ptr + 3) & ~3);
        level.staticMeshes = (const StaticMesh*)ptr;
        for (uint32 i = 0; i < level.staticMeshesCount; i++) {
            StaticMesh* s = (StaticMesh*)ptr; ptr += sizeof(StaticMesh);
            s->id = uint16(f.read32u()); s->meshIndex = f.read16u();
            s->vbox.minX = f.read16s(); s->vbox.maxX = f.read16s(); s->vbox.minY = f.read16s(); s->vbox.maxY = f.read16s(); s->vbox.minZ = f.read16s(); s->vbox.maxZ = f.read16s();
            s->cbox.minX = f.read16s(); s->cbox.maxX = f.read16s(); s->cbox.minY = f.read16s(); s->cbox.maxY = f.read16s(); s->cbox.minZ = f.read16s(); s->cbox.maxZ = f.read16s();
            s->flags = f.read16u();
        }
    }

    { // entities (items)
        level.itemsCount = f.read32u(); ptr = (uint8*)(((intptr_t)ptr + 3) & ~3);
        ItemObjInfo* item_base = (ItemObjInfo*)ptr; level.itemsInfo = item_base;
        ptr += level.itemsCount * sizeof(ItemObjInfo);
        for (uint32 i = 0; i < level.itemsCount; i++) {
            ItemObjInfo* item = item_base + i;
            item->type = (uint8)f.read16s(); item->roomIndex = (uint8)f.read16s();
            int32 ix = f.read32s(), iy = f.read32s(), iz = f.read32s();
            item->pos = _vec3s(ix, iy, iz); // Truncates to int16, but correct for struct
            f.read16s(); // angle
            item->intensity = f.read16s(); item->flags = f.read16u();
        }
    }

    rg_system_log(RG_LOG_INFO, "OpenLara", "read_PHD: DONE at ptr=%p (used %d bytes)\n", ptr, (int)(ptr - gLevelData));
    return true;
}
#endif
