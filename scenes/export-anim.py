# Reference export-scene.py and export-s72.py (https://github.com/amarantini/Vulkan-Render/blob/main/export-s72.py)

import sys,re

args = []
for i in range(0,len(sys.argv)):
	if sys.argv[i] == '--':
		args = sys.argv[i+1:]

if len(args) != 2:
	print("\n\nUsage:\nblender --background --python export-anim.py -- <infile.blend>[:collection] <outfile.anim>\nExports the animation of objects in collection (default: master collection) to a binary blob, indexed by the names of the objects that reference them.\n")
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


node_channels = dict()
for node, idx in obj_to_idx.items():
    node_channels[node] = ([], [], [])

times = []

for frame in range(frames[0], frames[1]+1):
    bpy.context.scene.frame_set(frame, subframe=0.0)
    time = (frame - frames[0]) / bpy.context.scene.render.fps
    times.append(f'{time:.3f}')
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

times = '[' + ','.join(times) + ']'
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

        out.append('{\n')
        out.append(f'\t"type":"DRIVER",\n')
        out.append(f'\t"name":{json.dumps(node.name + "-" + channel)},\n')
        out.append(f'\t"node":{idx},\n')
        out.append(f'\t"channel":{json.dumps(channel)},\n')
        out.append(f'\t"times":{times},\n')
        values = []
        for v in node_channels[node][c]:
            if c == 0 or c == 2:
                assert len(v) == 3
                values.append(f'{v.x:.6g},{v.y:.6g},{v.z:.6g}')
            elif c == 1:
                assert len(v) == 4
                values.append(f'{v.x:.6g},{v.y:.6g},{v.z:.6g},{v.w:.6g}')
            else:
                assert c < 3
        values = '[' + ', '.join(values) + ']'
        out.append(f'\t"values":{values},\n')
        if c == 1:
            out.append(f'\t"interpolation":"SLERP"\n')
        else:
            out.append(f'\t"interpolation":"LINEAR"\n')
        out.append('},\n')
