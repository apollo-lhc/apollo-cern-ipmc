import sys
from string import Template

ip_addr_map = {}
ip_addr_map[3] = "192.168.20.72"
ip_addr_map[5] = "192.168.20.56"

mac_addr_map = {}
mac_addr_map[3] = "00:50:51:FF:00:03"
mac_addr_map[5] = "00:50:51:FF:00:05"


def fill_template(template_file, params):
    template = Template(open(template_file, 'r').read())
    return template.substitute(params)
    
def main():
    params = {}
    params['ip_address'] = ip_addr_map[int(sys.argv[2])] 
    params['mac_address'] = mac_addr_map[int(sys.argv[2])] 

    with open('config.xml', 'w') as f:
        f.write(fill_template(sys.argv[1], params))

if __name__ == "__main__":
    main()
