import sys
import os
from string import Template
import subprocess
import stat
import yaml

PIPE = subprocess.PIPE

# get scripts directory path
scripts_dir = os.path.dirname(os.path.realpath(__file__))
base_dir = os.path.join(scripts_dir, "../")
sys.path.insert(0, base_dir)

def main():

    with open('config.yml') as f:
        # use safe_load instead load
        conf = yaml.safe_load(f)
    conf =  {k.lower(): v for k, v in conf.items()}

    with open(conf['network_file']) as f:
        network = yaml.safe_load(f)
    network =  {k.lower(): v for k, v in network.items()}

    params = {}

    revision = subprocess.run(["git", "describe", "--tags"], stdout=PIPE)
    revision = revision.stdout.decode().strip().split("-")
    
    # svn_cmd = 'svn info --show-item revision'.split()
    # revision = int(subprocess.check_output(svn_cmd).strip())

    params["major"] = revision[0]
    params["minor"] = revision[1]

    serial_conf = []
    if conf["sdi"] is True:
        serial_conf += ["<SerialIntf>SDI_INTF</SerialIntf>"]
        serial_conf += ["<RedirectSDItoSOL/>"]
    elif conf["sdi"] is False:
        serial_conf += ["<SerialIntf>SOL_INTF</SerialIntf>"]
    else:
        print("Unknown serial mode.")
        exit(1)
    params["serial"] = "\n    ".join(serial_conf)

    params["uart_target"] = conf["uart"]
    uart_conf = [] ## Inverted GPIO logic...
    if conf["uart"] == "zynq": # ADDR1=0, ADDR0=0
        uart_conf += ["<step>PSQ_ENABLE_SIGNAL(USER_IO_6)</step>"]
        uart_conf += ["<step>PSQ_ENABLE_SIGNAL(USER_IO_5)</step>"]
    elif conf["uart"] == "void":  # ADDR1=0, ADDR0=1
        uart_conf += ["<step>PSQ_ENABLE_SIGNAL(USER_IO_6)</step>"]
        uart_conf += ["<step>PSQ_DISABLE_SIGNAL(USER_IO_5)</step>"]
    elif conf["uart"] == "m1":  # ADDR1=1, ADDR0=0
        uart_conf += ["<step>PSQ_DISABLE_SIGNAL(USER_IO_6)</step>"]
        uart_conf += ["<step>PSQ_ENABLE_SIGNAL(USER_IO_5)</step>"]
    elif conf["uart"] == "m2": # ADDR1=1, ADDR0=1
        uart_conf += ["<step>PSQ_DISABLE_SIGNAL(USER_IO_6)</step>"]
        uart_conf += ["<step>PSQ_DISABLE_SIGNAL(USER_IO_5)</step>"]
    else:
        print("Unknown UART target.")
        exit(1)
    params["uart"] = "\n      ".join(uart_conf)


    params["gateway"] = network["gateway"]
    params["netmask"] = network["netmask"]

    dhcp_conf = []
    if conf["dhcp"] is True:
        dhcp_conf += ["<EnableDHCP />"]
    elif conf["dhcp"] is False:
        dhcp_conf += ["<!-- <EnableDHCP /> -->"]
    else:
        print("Unknown DHCP mode.")
        exit(1)
    params["dhcp"] = "".join(dhcp_conf)
    
        
    slot_ip_addresses = network.get("slot_ip_addr", {})
    params["slot_01_ip_addr"] = slot_ip_addresses.get( 1, "192.168.1.11")
    params["slot_02_ip_addr"] = slot_ip_addresses.get( 2, "192.168.1.12")
    params["slot_03_ip_addr"] = slot_ip_addresses.get( 3, "192.168.1.13")
    params["slot_04_ip_addr"] = slot_ip_addresses.get( 4, "192.168.1.14")
    params["slot_05_ip_addr"] = slot_ip_addresses.get( 5, "192.168.1.15")
    params["slot_06_ip_addr"] = slot_ip_addresses.get( 6, "192.168.1.16")
    params["slot_07_ip_addr"] = slot_ip_addresses.get( 7, "192.168.1.17")
    params["slot_08_ip_addr"] = slot_ip_addresses.get( 8, "192.168.1.18")
    params["slot_09_ip_addr"] = slot_ip_addresses.get( 9, "192.168.1.19")
    params["slot_10_ip_addr"] = slot_ip_addresses.get(10, "192.168.1.20")
    params["slot_11_ip_addr"] = slot_ip_addresses.get(11, "192.168.1.21")
    params["slot_12_ip_addr"] = slot_ip_addresses.get(12, "192.168.1.22")
    params["slot_13_ip_addr"] = slot_ip_addresses.get(13, "192.168.1.23")
    params["slot_14_ip_addr"] = slot_ip_addresses.get(14, "192.168.1.24")

    sm_id = conf["sm_id"]
    params["pn"] = "SM%05d" %(sm_id)
    params["sn"] = "%07d" %(sm_id)
    params["ip_addr"] = network["sm"][sm_id]["ip_addr"]
    params["mac_addr"] = network["sm"][sm_id]["mac_addr"]

    flashed_mac_addr = []
    if all(x == '00' for x in network["sm"][sm_id]["mac_addr"].split(":")):
        flashed_mac_addr += ["<UseFlashedMAC />"]
    else:
        flashed_mac_addr += ["<!-- <UseFlashedMAC /> -->"]        
    params["flashed_mac_addr"] = "".join(flashed_mac_addr)

    # find the proper template file to construct the config.xml
    templates_dir = os.path.join(base_dir, "templates")
    if conf["sm_id"] == 0:
        template_file = os.path.join(templates_dir, "config_devkit.xml.tpl")
    else:
        template_file = os.path.join(templates_dir, "config_sm.xml.tpl")

    target_file = os.path.join(scripts_dir, "../config.xml")
    if os.path.exists(target_file):
        os.chmod(target_file, stat.S_IWRITE )
    with open(target_file, 'w') as f:
        template = Template(open(template_file, 'r').read())
        f.write(template.substitute(params))

    os.chmod(target_file, stat.S_IREAD)

if __name__ == "__main__":
    main()
