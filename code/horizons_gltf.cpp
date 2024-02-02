#ifndef HORIZONS_GLTF_H
#define HORIZONS_GLTF_H

#include "json.h"

internal json_object_element_s *
FindJSONObjectElement(string8 Name, json_object_s *Object)
{
    json_object_element_s *Result = Object->start;
    for(memory_index ObjectIndex = 0;
        ObjectIndex < Object->length;
        ++ObjectIndex)
    {
        if(CompareStringBinary(Name, CreateString((char *)Result->name->string)))
        {
            return(Result);
        }
        
        Result = Result->next;
    }
    
    return(0);
}

internal json_array_element_s *
GrabJSONArrayElementIndexed(u32 Index, json_array_s *Array)
{
    json_array_element_s *Result = Array->start;
    for(u32 LoopIndex = 0;
        LoopIndex < Array->length;
        ++LoopIndex)
    {
        if(LoopIndex == Index)
        {
            return(Result);
        }
        
        Result = Result->next;
    }
    
    return(0);
}

internal json_array_s *
GrabJSONArray(string8 Name, json_object_s *Object)
{
    json_object_element_s *Element = FindJSONObjectElement(Name, Object);
    
    if(Element)
    {
        json_array_s *Result = json_value_as_array(Element->value);
        
        return(Result);
    }
    
    return(0);
}

typedef enum gltf_result
{
    GLTF_RESULT_SUCCESS,
    GLTF_RESULT_DOES_NOT_EXIST
} gltf_result;

internal gltf_result
GrabNumberFromJSONObject(string8 Name, json_object_s *Object, f32 *Result)
{
    json_object_element_s *Element = FindJSONObjectElement(Name, Object);
    
    if(!Element)
    {
        return(GLTF_RESULT_DOES_NOT_EXIST);
    }
    
    json_number_s *ASCII = json_value_as_number(Element->value);
    *Result = (f32)strtod(ASCII->number, 0);
    
    return(GLTF_RESULT_SUCCESS);
}

internal gltf_result
GrabNumberFromJSONArray(s32 Index, json_array_s *Array, f32 *Result)
{
    json_array_element_s *Element = Array->start;
    for(memory_index ElementIndex = 0;
        ElementIndex < Array->length;
        ++ElementIndex)
    {
        if(ElementIndex == Index)
        {
            break;
        }
        
        Element = Element->next;
    }
    
    if(!Element)
    {
        return(GLTF_RESULT_DOES_NOT_EXIST);
    }
    
    json_number_s *ASCII = json_value_as_number(Element->value);
    *Result = (f32)strtod(ASCII->number, 0);
    
    return(GLTF_RESULT_SUCCESS);
}

internal file_contents
GetGLTFBinaryInfo(u8 *Binary, u32 *ByteOffset, u32 *Count, string8 Name, b32 IsAttribute, memory_arena *Arena, string8 GLTFPath,
                  json_object_s *Attributes, json_object_s *Primitive, json_array_s *Accessors, json_array_s *BufferViews, json_array_s *Buffers)
{
    file_contents Result = {0};
    
    s32 Index;
    json_object_s *Accessor;
    if(IsAttribute)
    {
        f32 RealIndex = 0;
        if(GrabNumberFromJSONObject(Name, Attributes, &RealIndex) == GLTF_RESULT_DOES_NOT_EXIST)
        {
            return(Result);
        }
        
        Index = (s32)RealIndex;
    }
    else
    {
        f32 RealIndex = 0;
        if(GrabNumberFromJSONObject(Name, Primitive, &RealIndex) == GLTF_RESULT_DOES_NOT_EXIST)
        {
            return(Result);
        }
        
        Index = (s32)RealIndex;
    }
    
    json_array_element_s *AccessorElement = GrabJSONArrayElementIndexed(Index, Accessors);
    Accessor = json_value_as_object(AccessorElement->value);
    
    f32 RealBufferViewIndex = 0;
    if(GrabNumberFromJSONObject(CreateString("bufferView"), Accessor, &RealBufferViewIndex) == GLTF_RESULT_DOES_NOT_EXIST)
    {
        return(Result);
    }
    s32 BufferViewIndex = (s32)RealBufferViewIndex;
    
    f32 RealCount = 0;
    if(GrabNumberFromJSONObject(CreateString("count"), Accessor, &RealCount) == GLTF_RESULT_DOES_NOT_EXIST)
    {
        return(Result);
    }
    *Count = (u32)RealCount;
    
    json_array_element_s *BufferViewElement = GrabJSONArrayElementIndexed(BufferViewIndex, BufferViews);
    json_object_s *BufferView = json_value_as_object(BufferViewElement->value);
    
    f32 RealByteOffset = 0;
    if(GrabNumberFromJSONObject(CreateString("byteOffset"), BufferView, &RealByteOffset) == GLTF_RESULT_DOES_NOT_EXIST)
    {
        return(Result);
    }
    *ByteOffset = (u32)RealByteOffset;
    
    if(!Binary)
    {
        f32 RealBufferIndex = 0;
        if(GrabNumberFromJSONObject(CreateString("buffer"), BufferView, &RealBufferIndex) == GLTF_RESULT_DOES_NOT_EXIST)
        {
            return(Result);
        }
        s32 BufferIndex = (s32)RealBufferIndex;
        
        json_array_element_s *BufferElement = GrabJSONArrayElementIndexed(BufferIndex, Buffers);
        json_object_s *Buffer = json_value_as_object(BufferElement->value);
        
        json_object_element_s *BinNameElement = FindJSONObjectElement(CreateString("uri"), Buffer);
        json_string_s *BinNameJSONString = json_value_as_string(BinNameElement->value);
        string8 BinName = CreateString((char *)BinNameJSONString->string);
        char *BinPath = PushArray(Arena, char, GLTFPath.Length + BinName.Length + CreateString("/").Length);
        string8 BinPathString = CreateString(BinPath);
        GetPathNoFileName(&BinPathString, GLTFPath);
        StringConcat(&BinPathString, CreateString("/"));
        StringConcat(&BinPathString, BinName);
        
        Result = ReadFile8(BinPathString.String, Arena);
    }
    
    return(Result);
}

internal loaded_model
LoadGLTF(char *Path, memory_arena *Arena)
{
    /*
GLTF Spec:

Coordinate System:
The units for all linear distances are meters.
All angles are in radians.
Positive rotation is counterclockwise.

Component Types:
5120: BYTE
5121: UNSIGNED_BYTE
5122: SHORT
5123: UNSIGNED_SHORT
5125: UNSIGNED_INT
5126: FLOAT
*/
    
    // TODO(evan): Different nodes can access the same mesh
    // this would be a good optimization to make later
    
    loaded_model Result = {0};
    
    file_contents FileContents = ReadFile8(Path, &TranState->FrameArena);
    char *FileString = (char *)FileContents.Data;
    
    json_value_s *Root = json_parse(FileString, StringLength(FileString));
    json_object_s *Object = (json_object_s *)Root->payload;
    
    json_array_s *Nodes = GrabJSONArray(CreateString("nodes"), Object);
    json_array_s *Meshes = GrabJSONArray(CreateString("meshes"), Object);
    json_array_s *Accessors = GrabJSONArray(CreateString("accessors"), Object);
    json_array_s *BufferViews = GrabJSONArray(CreateString("bufferViews"), Object);
    json_array_s *Buffers = GrabJSONArray(CreateString("buffers"), Object);
    json_array_s *Materials = GrabJSONArray(CreateString("materials"), Object);
    
    u8 *Binary = 0;
    
    json_array_element_s *NodeElement = Nodes->start;
    while(true)
    {
        json_object_s *Node = (json_object_s *)NodeElement->value->payload;
        
        f32 RealMeshIndex = 0;
        Assert(GrabNumberFromJSONObject(CreateString("mesh"), Node, &RealMeshIndex) == GLTF_RESULT_SUCCESS);
        s32 MeshIndex = (s32)RealMeshIndex;
        
        // NOTE(evan): Scale
        v3 Scale = { 1.0f, 1.0f, 1.0f };
        {
            json_object_element_s *ScaleElement = FindJSONObjectElement(CreateString("scale"), Node);
            if(ScaleElement)
            {
                json_array_s *ScaleArray = json_value_as_array(ScaleElement->value);
                
                GrabNumberFromJSONArray(0, ScaleArray, &Scale.x);
                GrabNumberFromJSONArray(1, ScaleArray, &Scale.y);
                GrabNumberFromJSONArray(2, ScaleArray, &Scale.z);
            }
        }
        
        json_array_element_s *MeshArrayElement = GrabJSONArrayElementIndexed(MeshIndex, Meshes);
        json_object_s *MeshObject = json_value_as_object(MeshArrayElement->value);
        
        json_object_element_s *PrimitivesElement = FindJSONObjectElement(CreateString("primitives"), MeshObject);
        json_array_s *Primitives = json_value_as_array(PrimitivesElement->value);
        json_array_element_s *PrimitiveElement = Primitives->start;
        
        Result.Meshes = PushStruct(Arena, mesh);
        mesh *Mesh = Result.Meshes;
        Result.MeshCount = 1;
        while(true)
        {
            Mesh->RelScale = Scale;
            
            json_object_s *Primitive = (json_object_s *)PrimitiveElement->value->payload;
            
            json_object_element_s *AttributesElement = FindJSONObjectElement(CreateString("attributes"), Primitive);
            json_object_s *Attributes = json_value_as_object(AttributesElement->value);
            
            string8 PathString = CreateString(Path);
            
            //
            // NOTE(evan): Positions
            //
            {
                u32 ByteOffset = 0;
                u32 Count = 0;
                file_contents Contents = GetGLTFBinaryInfo(Binary, &ByteOffset, &Count, CreateString("POSITION"), true,
                                                           &TranState->FrameArena, PathString,
                                                           Attributes, Primitive, Accessors, BufferViews, Buffers);
                Binary = (u8 *)Contents.Data;
                
                v3 *Positions = (v3 *)(Binary + ByteOffset);
                
                Mesh->Vertices = PushArray(Arena, vertex, Count);
                Mesh->VertexCount = Count;
                
                for(u32 VertexIndex = 0;
                    VertexIndex < Count;
                    ++VertexIndex)
                {
                    v3 Position = Positions[VertexIndex];
                    Mesh->Vertices[VertexIndex].Position = Position;
                }
                
                Platform->CloseFile(Contents.Handle);
            }
            
            //
            // NOTE(evan): Indices
            //
            {
                u32 ByteOffset = 0;
                u32 Count = 0;
                file_contents Contents = GetGLTFBinaryInfo(Binary, &ByteOffset, &Count, CreateString("indices"), false,
                                                           &TranState->FrameArena, PathString,
                                                           Attributes, Primitive, Accessors, BufferViews, Buffers);
                
                u16 *Indices = (u16 *)(Binary + ByteOffset);
                Mesh->Indices = PushArray(Arena, u32, Count);
                Mesh->IndexCount = Count;
                for(u32 IndIndex = 0;
                    IndIndex < Count;
                    ++IndIndex)
                {
                    Mesh->Indices[IndIndex] = (u32)(Indices[IndIndex]);
                }
                
                Platform->CloseFile(Contents.Handle);
            }
            
            //
            // NOTE(evan): Material
            //
            if(Materials)
            {
                f32 RealIndex;
                Assert(GrabNumberFromJSONObject(CreateString("material"), Primitive, &RealIndex) == GLTF_RESULT_SUCCESS);
                s32 Index = (s32)RealIndex;
                
                json_array_element_s *MaterialElement = GrabJSONArrayElementIndexed(Index, Materials);
                json_object_s *Material = json_value_as_object(MaterialElement->value);
                
                json_object_element_s *PBRMetallicRoughnessElement =
                    FindJSONObjectElement(CreateString("pbrMetallicRoughness"), Material);
                json_object_s *PBRMetallicRoughness = json_value_as_object(PBRMetallicRoughnessElement->value);
                
                json_object_element_s *BaseColorArrayElement = FindJSONObjectElement(CreateString("baseColorFactor"), PBRMetallicRoughness);
                json_array_s *BaseColorArray = json_value_as_array(BaseColorArrayElement->value);
                
                f32 R, G, B, A;
                Assert(GrabNumberFromJSONArray(0, BaseColorArray, &R) == GLTF_RESULT_SUCCESS);
                Assert(GrabNumberFromJSONArray(1, BaseColorArray, &G) == GLTF_RESULT_SUCCESS);
                Assert(GrabNumberFromJSONArray(2, BaseColorArray, &B) == GLTF_RESULT_SUCCESS);
                Assert(GrabNumberFromJSONArray(3, BaseColorArray, &A) == GLTF_RESULT_SUCCESS);
                Mesh->Material.BaseColor = V4(R, G, B, A);
            }
            else
            {
                Mesh->Material.BaseColor = COLOR_WHITE;
            }
            
            //
            // NOTE(evan): Normals
            //
            {
                u32 ByteOffset = 0;
                u32 Count = 0;
                file_contents Contents = GetGLTFBinaryInfo(Binary, &ByteOffset, &Count, CreateString("NORMAL"), true,
                                                           &TranState->FrameArena, PathString,
                                                           Attributes, Primitive, Accessors, BufferViews, Buffers);
                
                v3 *Normals = (v3 *)(Binary + ByteOffset);
                
                for(u32 VertexIndex = 0;
                    VertexIndex < Mesh->VertexCount;
                    ++VertexIndex)
                {
                    Mesh->Vertices[VertexIndex].Normal = Normals[VertexIndex];
                }
                
                Platform->CloseFile(Contents.Handle);
            }
            
            //
            // NOTE(evan): UVs
            //
            {
                u32 ByteOffset = 0;
                u32 Count = 0;
                file_contents Contents = GetGLTFBinaryInfo(Binary, &ByteOffset, &Count, CreateString("TEXCOORD_0"), true,
                                                           &TranState->FrameArena, PathString,
                                                           Attributes, Primitive, Accessors, BufferViews, Buffers);
                
                v2 *UVs = (v2 *)(Binary + ByteOffset);
                
                for(u32 VertexIndex = 0;
                    VertexIndex < Count;
                    ++VertexIndex)
                {
                    Mesh->Vertices[VertexIndex].UV = UVs[VertexIndex];
                }
                
                Platform->CloseFile(Contents.Handle);
            }
            
            //
            // NOTE(evan): Vertex Colors
            //
            {
                u32 ByteOffset = 0;
                u32 Count = 0;
                file_contents Contents = GetGLTFBinaryInfo(Binary, &ByteOffset, &Count, CreateString("COLOR_0"), true,
                                                           &TranState->FrameArena, PathString,
                                                           Attributes, Primitive, Accessors, BufferViews, Buffers);
                
                if(Count)
                {
                    v4 *Colors = (v4 *)(Binary + ByteOffset);
                    
                    for(u32 VertexIndex = 0;
                        VertexIndex < Mesh->VertexCount;
                        ++VertexIndex)
                    {
                        Mesh->Vertices[VertexIndex].Color = Colors[VertexIndex];
                    }
                    
                    Platform->CloseFile(Contents.Handle);
                }
                else
                {
                    for(u32 VertexIndex = 0;
                        VertexIndex < Mesh->VertexCount;
                        ++VertexIndex)
                    {
                        Mesh->Vertices[VertexIndex].Color = V4(1.0f, 1.0f, 1.0f, 1.0f);
                    }
                }
            }
            
            Mesh->Platform = Platform->CreateMesh(Mesh->Vertices, Mesh->VertexCount, Mesh->Indices, Mesh->IndexCount, false);
            
            if(PrimitiveElement->next == 0)
            {
                break;
            }
            
            PrimitiveElement = PrimitiveElement->next;
            Mesh->Next = PushStruct(Arena, mesh);
            Mesh = Mesh->Next;
            ++Result.MeshCount;
        }
        
        if(NodeElement->next == 0)
        {
            break;
        }
        NodeElement = NodeElement->next;
    }
    
    Platform->CloseFile(FileContents.Handle);
    
    return(Result);
}

#endif //HORIZONS_GLTF_H
