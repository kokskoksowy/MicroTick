#include <Arduino.h>
#include <vector>
#include <map>
#include <cstdint>
#include <registries/registries.h>
#include <MinecraftFunctions.h>
std::vector<Chunk> chunks(CHUNK_COUNT);




void InitChunksAroundZero() {
    uint32_t start = millis();
    int index = 0;

    for (int cz = -CHUNK_RADIUS; cz <= CHUNK_RADIUS; cz++) {
        for (int cx = -CHUNK_RADIUS; cx <= CHUNK_RADIUS; cx++) {
            Chunk &chunk = chunks[index++];
            chunk.posX = cx;
            chunk.posZ = cz;
            chunk.tickingTime = 0;

            for (int i = 0; i < CHUNK_VOLUME; i++) {
                int y = i / (CHUNK_SIZE_X * CHUNK_SIZE_Z); 

                if (y == 0) chunk.data[i] = B_bedrock;
                else if (y >= 1 && y <= 58) chunk.data[i] = B_stone;
                else if (y >= 59 && y <= 63) chunk.data[i] = B_dirt;
                else if (y == 64) chunk.data[i] = B_grass_block;
                else chunk.data[i] = 0;
            }
        }
    }

    Serial.print("Creating chunks took: ");
    Serial.print(millis() - start);
    Serial.println("ms.");
}



Chunk* getChunk(int32_t x, int32_t z) {
    for (auto &chunk : chunks) {
        if (chunk.posX == x && chunk.posZ == z)
            return &chunk;
    }
    return nullptr;
}
