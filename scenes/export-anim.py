# Reference export-scene.py and export-s72.py (https://github.com/amarantini/Vulkan-Render/blob/main/export-s72.py)

import sys,re

args = []
for i in range(0,len(sys.argv)):
    if sys.argv[i] == '--':
        args = sys.argv[i+1:]

def usage():
    print("\n\nUsage:\nblender --background --python export-anim.py -- <infile.blend>[:collection] <outfile.anim> --animate 0 60\nExports the animation of objects in collection (default: master collection) to a binary blob, indexed by the names of the objects that reference them.\n")
    exit(1)
    
infile = args[0]
collection_name = None
m = re.match(r'^(.*?):(.+)$', infile)
if m:
    infile = m.group(1)
    collection_name = m.group(2)
outfile = args[1]

print("Will transforms of objects in ",end="")
if collection_name:
    print("collection '" + collection_name + "'",end="")
else:
    print('master collection',end="")
print(" of '" + infile + "' to '" + outfile + "'.")

i = 0
while i < len(args):
    arg = args[i]
    if arg.startswith('--'):
        if arg == '--animate':
            if i + 2 >= len(args):
                print("ERROR: --animate must be followed by a min and max frame number.")
                usage()
            frames = (int(args[i+1]), int(args[i+2]))

            i += 2
        else:
            print(f"ERROR: unrecognized argument '{arg}'.")
            usage()
    i += 1

#---------------------------------------------------------------------
#Export animation:
import bpy
import mathutils
import struct
import math

bpy.ops.wm.open_mainfile(filepath=infile)

if collection_name:
    if not collection_name in bpy.data.collections:
        print("ERROR: Collection '" + collection_name + "' does not exist in scene.")
        exit(1)
    collection = bpy.data.collections[collection_name]
else:
    collection = bpy.context.scene.collection

fresh_idx = 1
obj_to_idx = dict()

"""
Animation file format:
int0 len <uint> [number of aniamtions]

str0 len < char > * [transform name - channel]
time len < float > * [time values]
val3 len < float > * [translation/scale values]
or
val4 len < float > * [rotation values]
"""

     
# get nodes
def get_node(obj, extra_children=[]):
    global out, fresh_idx, obj_to_idx
    print(obj.name)

    if obj in obj_to_idx:
        assert obj_to_idx[obj] != None #no cycles!
        return obj_to_idx[obj]

    obj_to_idx[obj] = None
    
    children = []

    if obj.name.startswith("!environment:"):
        return
    else:

        for child in obj.children:
            children.append(get_node(child))

        if obj.type == 'MESH':
            return
        elif obj.type == 'CAMERA':
            return
        elif obj.type == 'LIGHT':
            return
        elif obj.type == 'EMPTY':
            if obj.instance_collection:
                children += get_nodes(obj.instance_collection)
        else:
            print(f"ignoring object data of type '{obj.type}'.")

    idx = fresh_idx
    fresh_idx += 1
    obj_to_idx[obj] = idx

    if obj.parent == None:
        parent_from_world = mathutils.Matrix()
    else:
        parent_from_world = obj.parent.matrix_world.copy()
        parent_from_world.invert()
    
    (t, r, s) = (parent_from_world @ obj.matrix_world).decompose()

    return idx

def get_nodes(from_collection):
    roots = []
    global obj_to_idx
    for obj in from_collection.objects:
        #has a parent, will be emitted by write_node(parent)
        if obj.parent: continue
        get_node(obj)

    #handle nested collections:
    for child in from_collection.children:
        get_nodes(child)
    
    return roots

# def write_string(string):
#     string_data = b""
#     begin = len(strings_data)
#     strings_data += bytes(string, 'utf8')
#     end = len(strings_data)
#     return struct.pack('II', begin, end)

get_nodes(collection)
print("Done traversing scene graph.")
node_channels = dict()
for node, idx in obj_to_idx.items():
    node_channels[node] = ([], [], [])

times = []

for frame in range(frames[0], frames[1]+1):
    bpy.context.scene.frame_set(frame, subframe=0.0)
    time = (frame - frames[0]) / bpy.context.scene.render.fps
    times.append(time)
    for node, idx in obj_to_idx.items():
        if node.parent == None:
            parent_from_world = mathutils.Matrix()
        else:
            parent_from_world = node.parent.matrix_world.copy()
            parent_from_world.invert()

        (t, r, s) = (parent_from_world @ node.matrix_world).decompose()
        node_channels[node][0].append(t)
        node_channels[node][1].append(r)
        node_channels[node][2].append(s)
print("Done collecting animation data.")

anim = dict()

for node, idx in obj_to_idx.items():
    for c in range(0,3):
        driven = False
        if c == 0 or c == 2:
            for v in node_channels[node][c]:
                if (v - node_channels[node][c][0]).length > 0.0001:
                    driven = True
        elif c == 1:
            for v in node_channels[node][c]:
                if v.rotation_difference(node_channels[node][c][0]).angle > 0.0001:
                    driven = True
        if not driven: continue
        channel = ["translation", "rotation", "scale"][c]
        print(f"Writing \"{channel}\" driver for '{node.name}'.")

        values = []
        for v in node_channels[node][c]:
            if c == 0 or c == 2:
                assert len(v) == 3
                values.append(v)
            elif c == 1:
                assert len(v) == 4
                values.append(v)
            else:
                assert c < 3
        anim[node.name + "-" + channel] = (times, values)



#write the strings chunk and scene chunk to an output blob:
blob = open("../dist/"+outfile, 'wb')
def write_chunk(magic, data):
    blob.write(struct.pack('4s',magic)) #type
    blob.write(struct.pack('I', len(data))) #length
    blob.write(data)

write_chunk(b'int0', struct.pack('i', len(anim)))
for anim_name, (times, values) in anim.items():
    string_data = bytes(anim_name, 'utf8')
    write_chunk(b'str0', string_data)
    time_data = b""
    for time in times:
        time_data += struct.pack('f', time)
    write_chunk(b'time', time_data)
    values_data = b""
    for value in values:
        if len(value) == 3:
            values_data += struct.pack('3f', value.x, value.y, value.z)
        else:
            values_data += struct.pack('4f', value.x, value.y, value.z, value.w)
    if len(values[0]) == 3:
        write_chunk(b'val3', values_data)
    elif len(values[0]) == 4:
        write_chunk(b'val4', values_data)

print("Wrote " + str(blob.tell()) + " bytes to '" + outfile + "'")
blob.close()