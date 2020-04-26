import os
import subprocess

asset_table = [
        ('gSoundDataADSR', ['sound','sound_data.ctl']),
        ('gSoundDataRaw',  ['sound','sound_data.tbl']),
        ('gMusicData',     ['sound','sequences.bin']),
        ('gBankSetsData',  ['sound','bank_sets']),
        ]

sky_table = [
        'ccm', 'bbh', 'bidw', 'cloud_floor', 'clouds', 'wdw', 'ssl', 'water', 'bits', 'bitfs'
        ]

def create_c_asset(in_file, out_file, name):
    # read binary file
    f = open(in_file, mode='rb')
    data = f.read()

    # write C file
    with open(out_file, mode='w') as o:
        o.write("#include <ultra64.h>\n")
        o.write("u8 {}[] = {{\n".format(name))
        for i in range(len(data)):
            o.write("0x{:02X}, ".format(data[i]))
            if ((i + 1) % 32) == 0:
                o.write("\n")
        o.write("};\n")

def main():
    # create directory
    os.makedirs("pc_assets", exist_ok=True)

    decomp_build = ['build', 'us']

    # create sound data
    for asset_name, file_path in asset_table:
        in_path = os.path.join(*(decomp_build + file_path))
        out_path = os.path.join('pc_assets', asset_name + '.c')
        create_c_asset(in_path, out_path, asset_name)

    # create sky stuff
    for sky in sky_table:
        subprocess.call([os.path.join('tools','skyconv'), '--type', 'sky', '--split', os.path.join('textures', 'skyboxes', sky + '.png'), 'pc_assets'])


if __name__ == "__main__":
    main()
