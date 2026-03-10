# Stagehand ECS Patterns & Best Practices

Stagehand introduces facilities and patterns to streamline working with components.

## Defining and Registering Components

Through macros, boilerplate-free component definition and registration of components is possible. The macros also handle the per-component setup of [automatic change detection](ChangeDetection.md) behind the scenes.


### Manually defined & registered components


### Primitive C++ Types

#### `FLOAT(Name, ...)` / `FLOAT_(Name, ...)` - 32-bit (single precision) floating point number
```cpp
FLOAT(Health); // Default-initialised to 0.0f.
FLOAT(MaxHealth, 100.0f); // Initialised with a default value.
```

#### `DOUBLE(Name, ...)` / `DOUBLE_(Name, ...)` - 64-bit (double precision) floating point number
```cpp
DOUBLE(Gravity); // Default-initialised to 0.0.
DOUBLE(Pi, 3.14159265359); // Initialised with a default value.
```

#### `INT8(Name, ...)` / `INT8_(Name, ...)` - 8-bit signed integer
```cpp
INT8(Level); // Default-initialised to 0.
INT8(MaxLevel, 99); // Initialised with a default value.
```

#### `UINT8(Name, ...)` / `UINT8_(Name, ...)` - 8-bit unsigned integer
```cpp
UINT8(Lives); // Default-initialised to 0.
UINT8(MaxLives, 3); // Initialised with a default value.
```

#### `INT16(Name, ...)` / `INT16_(Name, ...)` - 16-bit signed integer
```cpp
INT16(Damage); // Default-initialised to 0.
INT16(BaseDamage, 10); // Initialised with a default value.
```

#### `UINT16(Name, ...)` / `UINT16_(Name, ...)` - 16-bit unsigned integer
```cpp
UINT16(Ammo); // Default-initialised to 0.
UINT16(MaxAmmo, 250); // Initialised with a default value.
```

#### `INT32(Name, ...)` / `INT32_(Name, ...)` - 32-bit signed integer
```cpp
INT32(Score); // Default-initialised to 0.
INT32(HighScore, 10000); // Initialised with a default value.
```

#### `UINT32(Name, ...)` / `UINT32_(Name, ...)` - 32-bit unsigned integer
```cpp
UINT32(Gold); // Default-initialised to 0.
UINT32(StartGold, 50); // Initialised with a default value.
```

#### `INT64(Name, ...)` / `INT64_(Name, ...)` - 64-bit signed integer
```cpp
INT64(ParticleCount); // Default-initialised to 0.
INT64(AVeryLowInteger, -135761385678932632); // Initialised with a default value.
```

#### `UINT64(Name, ...)` / `UINT64_(Name, ...)` - 64-bit unsigned integer
```cpp
UINT64(IDontEvenKnowWhenIWouldUseThis); // Default-initialised to 0.
UINT64(AVeryHighInteger, 53925713520814); // Initialised with a default value.
```

### Other C++ Types

#### `TAG(Name)` / `TAG_(Name)`
```cpp
TAG(IsPlayer); // Defines a tag component.
TAG_(IsGrounded); // Alias for TAG, no change detection applicable for tags.
```

#### `ENUM(Name, [UnderlyingType])` / `ENUM_(Name, [UnderlyingType])`
```cpp
enum class AIState { Idle, Patrol, Chase };
ENUM(AIState); // Defaults to uint8_t underlying type.

enum class WeaponType : uint16_t { Sword, Axe, Bow };
ENUM(WeaponType, uint16_t); // Specifies underlying type.

enum class Weather { Sunny, Rainy };
ENUM_(Weather); // Opt out of change detection.
```

#### `POINTER(Name, Type, [DefaultValue])` / `POINTER_(Name, Type, [DefaultValue])`
```cpp
POINTER(CurrentTarget, Targetable); // Default-initialised to nullptr.
POINTER(GameWindow, void, nullptr); // Initialised with an explicit default value.
```

#### `VECTOR(Name, ElementType, [Initialiser])` / `VECTOR_(Name, ElementType, [Initialiser])`
```cpp
VECTOR(PathNodes, godot::Vector3); // Default-initialised to an empty vector.
VECTOR(ItemIDs, int, {10, 23, 42}); // Initialised with a default list.
```

#### `ARRAY(Name, ElementType, Size, [Initialiser])` / `ARRAY_(Name, ElementType, Size, [Initialiser])`
```cpp
ARRAY(InputBuffer, uint8_t, 8); // An array of 8 elements that are default-initialised.
ARRAY(ModelView, float, 4, {1,0,0,1}); // Initialised with a default list.
```

#### `STRUCT(Name, { ... })` / `STRUCT_(Name, { ... })`
```cpp
STRUCT(PlayerStats, { float age; int rank; }); // Members are default-initialised.
STRUCT(ColoredPoint, { godot::Vector3 position = godot::Vector3(0, 0, 0); godot::Color color = godot::Color(1, 1, 1); }); // Members initialised with default values.
STRUCT_(TrackedPosition, { float x = 0.0f; float y = 0.0f; float z = 0.0f; }); // Opt out of change detection.
```

### Godot Variants

#### `GODOT_VARIANT(Name, Type, [DefaultValue])` / `GODOT_VARIANT_(Name, Type, [DefaultValue])`

These macros wrap Godot's built-in Variant types as components. They support both struct-based (Plain Old Data) types and class-based (handle) types.

#### Struct-based (Plain Old Data) Variants
```cpp
// Color
GODOT_VARIANT(BackgroundColor, godot::Color); // Default-initialised.
GODOT_VARIANT(TintColor, godot::Color, godot::Color(1, 1, 1, 1)); // With default value.

// Vector2
GODOT_VARIANT(Velocity2D, godot::Vector2);
GODOT_VARIANT(SpriteScale, godot::Vector2, godot::Vector2(1, 1));

// Vector2i
GODOT_VARIANT(TilePosition, godot::Vector2i);
GODOT_VARIANT(GridCoordinates, godot::Vector2i, godot::Vector2i(0, 0));

// Vector3
GODOT_VARIANT(Velocity3D, godot::Vector3);
GODOT_VARIANT(Position, godot::Vector3, godot::Vector3(0, 0, 0));

// Vector3i
GODOT_VARIANT(VoxelPosition, godot::Vector3i);
GODOT_VARIANT(ChunkCoordinate, godot::Vector3i, godot::Vector3i(0, 0, 0));

// Vector4
GODOT_VARIANT(PlaneEquation, godot::Vector4);
GODOT_VARIANT(CustomData, godot::Vector4, godot::Vector4(1, 0, 0, 1));

// Vector4i
GODOT_VARIANT(TilemapRect, godot::Vector4i);
GODOT_VARIANT(IntegerQuad, godot::Vector4i, godot::Vector4i(0, 0, 100, 100));

// Rect2
GODOT_VARIANT(BoundingBox, godot::Rect2);
GODOT_VARIANT(ViewportRect, godot::Rect2, godot::Rect2(0, 0, 800, 600));

// Rect2i
GODOT_VARIANT(TextureRegion, godot::Rect2i);
GODOT_VARIANT(AtlasRegion, godot::Rect2i, godot::Rect2i(0, 0, 64, 64));

// Plane
GODOT_VARIANT(ClippingPlane, godot::Plane);
GODOT_VARIANT(MirrorPlane, godot::Plane, godot::Plane(godot::Vector3(0, 1, 0), 0));

// Quaternion
GODOT_VARIANT(Rotation, godot::Quaternion);
GODOT_VARIANT(TargetRotation, godot::Quaternion, godot::Quaternion(0, 0, 0, 1));

// Basis
GODOT_VARIANT(Orientation, godot::Basis);
GODOT_VARIANT(CoordinateFrame, godot::Basis, godot::Basis());

// Transform2D
GODOT_VARIANT(SpriteTransform, godot::Transform2D);
GODOT_VARIANT(CanvasTransform, godot::Transform2D, godot::Transform2D());

// Transform3D
GODOT_VARIANT(WorldTransform, godot::Transform3D);
GODOT_VARIANT(BoneTransform, godot::Transform3D, godot::Transform3D());

// AABB
GODOT_VARIANT(CollisionAABB, godot::AABB);
GODOT_VARIANT(SelectionBox, godot::AABB, godot::AABB(godot::Vector3(-1, -1, -1), godot::Vector3(2, 2, 2)));

// Projection
GODOT_VARIANT(CameraProjection, godot::Projection);
GODOT_VARIANT(CustomProjection, godot::Projection, godot::Projection());
```

#### Class-based (handle) Variants
```cpp
// Array
GODOT_VARIANT(ChildNodes, godot::Array);
GODOT_VARIANT(InventoryItems, godot::Array, godot::Array());

// PackedByteArray
GODOT_VARIANT(SerializedData, godot::PackedByteArray);
GODOT_VARIANT(CompressedData, godot::PackedByteArray, godot::PackedByteArray());

// PackedColorArray
GODOT_VARIANT(GradientColors, godot::PackedColorArray);
GODOT_VARIANT(PaletteColors, godot::PackedColorArray, godot::PackedColorArray());

// PackedFloat32Array
GODOT_VARIANT(VertexPositions, godot::PackedFloat32Array);
GODOT_VARIANT(AudioSamples, godot::PackedFloat32Array, godot::PackedFloat32Array());

// PackedFloat64Array
GODOT_VARIANT(HighPrecisionData, godot::PackedFloat64Array);
GODOT_VARIANT(SimulationResults, godot::PackedFloat64Array, godot::PackedFloat64Array());

// PackedInt32Array
GODOT_VARIANT(TileIndices, godot::PackedInt32Array);
GODOT_VARIANT(MeshIndices, godot::PackedInt32Array, godot::PackedInt32Array());

// PackedInt64Array
GODOT_VARIANT(EntityHandles, godot::PackedInt64Array);
GODOT_VARIANT(TimestampList, godot::PackedInt64Array, godot::PackedInt64Array());

// PackedStringArray
GODOT_VARIANT(DialogueLines, godot::PackedStringArray);
GODOT_VARIANT(CommandHistory, godot::PackedStringArray, godot::PackedStringArray());

// PackedVector2Array
GODOT_VARIANT(UVCoordinates, godot::PackedVector2Array);
GODOT_VARIANT(PathPoints2D, godot::PackedVector2Array, godot::PackedVector2Array());

// PackedVector3Array
GODOT_VARIANT(MeshVertices, godot::PackedVector3Array);
GODOT_VARIANT(WaypointList, godot::PackedVector3Array, godot::PackedVector3Array());

// PackedVector4Array
GODOT_VARIANT(ParticleColors, godot::PackedVector4Array);
GODOT_VARIANT(TangentData, godot::PackedVector4Array, godot::PackedVector4Array());

// Dictionary
GODOT_VARIANT(GameSettings, godot::Dictionary);
GODOT_VARIANT(Metadata, godot::Dictionary, godot::Dictionary());

// String
GODOT_VARIANT(Description, godot::String);
GODOT_VARIANT(ObjectName, godot::String, "Unnamed");

// StringName
GODOT_VARIANT(AnimationState, godot::StringName);
GODOT_VARIANT(ActionName, godot::StringName, "jump");

// NodePath
GODOT_VARIANT(TargetPath, godot::NodePath);
GODOT_VARIANT(CameraPath, godot::NodePath, "Player/Camera");

// Callable
GODOT_VARIANT(OnDeathCallback, godot::Callable);
GODOT_VARIANT(UpdateFunction, godot::Callable, godot::Callable());

// Signal
GODOT_VARIANT(HealthChangedSignal, godot::Signal);
GODOT_VARIANT(CustomSignal, godot::Signal, godot::Signal());

// RID
GODOT_VARIANT(PhysicsBodyRID, godot::RID);
GODOT_VARIANT(MaterialRID, godot::RID, godot::RID());
```
