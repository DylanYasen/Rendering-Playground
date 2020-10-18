
#include <unordered_map>

enum class EResourceType
{
    Generic = 0,
    Texture = 1,
    Geometry = 2,
    Max = 3
};

static const char *ResourceTypeToStr(EResourceType e)
{
    switch (e)
    {
    case EResourceType::Generic:
        return "Generic";

    case EResourceType::Texture:
        return "Texture";

    case EResourceType::Geometry:
        return "Geometry";

    default:
        return "N/A";
    }
}

class MemTracker
{

public:
    struct Chunk
    {
        Chunk() = default;
        Chunk(size_t s, EResourceType t) : size(s), type(t){};

        size_t size;
        EResourceType type;
    };

    static void track(const void *p, EResourceType type, size_t size);
    static void track(uint16_t resourceID, EResourceType type, size_t size);

    static void untrack(void *p);
    static void untrack(uint16_t resourceID);

    static std::unordered_map<uint8_t, size_t> resourceMap;

    // todo: resource id could collide.
    static std::unordered_map<uint16_t, Chunk> resourceIDMap;
    static std::unordered_map<uintptr_t, Chunk> addressMap;
};