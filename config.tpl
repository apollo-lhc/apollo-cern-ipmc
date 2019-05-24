<?xml version="1.0" encoding="UTF-8"?>

<IPMC>
	<GeneralConfig>

		<DeviceID>0x12</DeviceID>
		<DeviceRevision>0x00</DeviceRevision>
		<ManufacturerID>0x000060</ManufacturerID>
		<ProductID>0x1236</ProductID>
		
		<ManufacturingDate>06/01/2017</ManufacturingDate>
		
		<BoardManuf>Cirly/Addax</BoardManuf>
		<BoardName>TEST_FRUFROLLBACK</BoardName>
		<BoardSN>00001</BoardSN>
		<BoardPN>P580050995</BoardPN>
		
		<ProductManuf>CERN</ProductManuf>
		<ProductName>IPMC-TestPAD</ProductName>
		<ProductPN>PN00001</ProductPN>
		<ProductSN>0000001</ProductSN>
		<ProductVersion type="major">1</ProductVersion>	
		<ProductVersion type="minor">20</ProductVersion>			
				
		<MaxCurrent>10.0</MaxCurrent>
		<MaxInternalCurrent>1.0</MaxInternalCurrent>
		
		<!-- Hardware -->
		<HandleSwitch active="LOW" inactive="HIGH" />
        
        <!-- <ResetOnWrongHAEn /> -->
        <!-- <PowerMonitoringEn /> -->
        <!-- <AlertMonitoringEn />-->
        
		<SerialIntf>SDI_INTF</SerialIntf>
		<RedirectSDItoSOL/>
		
	</GeneralConfig>
		
	<PowerManagement>
		
		<PowerONSeq>
			<step>PSQ_ENABLE_SIGNAL(CFG_PAYLOAD_DCDC_EN_SIGNAL)</step>
			<step>PSQ_END</step>
		</PowerONSeq>
		
		<PowerOFFSeq>
			<step>PSQ_DISABLE_SIGNAL(CFG_PAYLOAD_DCDC_EN_SIGNAL)</step>
			<step>PSQ_END</step>
		</PowerOFFSeq>
		
	</PowerManagement>
	
	<LANConfig>	
	
		<MACAddr>0A:0A:0A:0A:0A:CC</MACAddr>
		<NetMask>255.255.0.0</NetMask>
		<GatewayIP>192.168.1.3</GatewayIP>
		
        <!-- <EnableDHCP /> -->
        
		<IPAddrList> <!-- Default IP Addresses (used if DHCP is not active) -->
			<IPAddr slot_addr="default">192.168.1.34</IPAddr>
			<IPAddr slot_addr="0x41">192.168.1.20</IPAddr>
			<IPAddr slot_addr="0x42">192.168.1.21</IPAddr>
			<IPAddr slot_addr="0x43">192.168.1.22</IPAddr>
			<IPAddr slot_addr="0x44">192.168.1.23</IPAddr>
			<IPAddr slot_addr="0x45">192.168.1.24</IPAddr>
			<IPAddr slot_addr="0x46">192.168.1.25</IPAddr>
			<IPAddr slot_addr="0x47">192.168.1.26</IPAddr>
			<IPAddr slot_addr="0x48">192.168.1.27</IPAddr>
			<IPAddr slot_addr="0x49">192.168.1.28</IPAddr>
			<IPAddr slot_addr="0x4a">192.168.1.29</IPAddr>
			<IPAddr slot_addr="0x4b">192.168.1.30</IPAddr>
			<IPAddr slot_addr="0x4c">192.168.1.31</IPAddr>
			<IPAddr slot_addr="0x4d">192.168.1.32</IPAddr>
			<IPAddr slot_addr="0x4e">192.168.1.33</IPAddr>
		</IPAddrList>
		
	</LANConfig>	
<!--	
	<AMCSlots>

		<AMC site="1">
			<PhysicalPort>1</PhysicalPort>
			<MaxCurrent>6.0</MaxCurrent>
			<PowerGoodTimeout>300</PowerGoodTimeout>
			<DCDCEfficiency>85</DCDCEfficiency>
		</AMC>

		<AMC site="2">
			<PhysicalPort>2</PhysicalPort>
			<MaxCurrent>6.0</MaxCurrent>
			<PowerGoodTimeout>300</PowerGoodTimeout>
			<DCDCEfficiency>85</DCDCEfficiency>
		</AMC>

		<AMC site="3">
			<PhysicalPort>3</PhysicalPort>
			<MaxCurrent>6.0</MaxCurrent>
			<PowerGoodTimeout>300</PowerGoodTimeout>
			<DCDCEfficiency>85</DCDCEfficiency>
		</AMC>

		<AMC site="4">
			<PhysicalPort>4</PhysicalPort>
			<MaxCurrent>6.0</MaxCurrent>
			<PowerGoodTimeout>300</PowerGoodTimeout>
			<DCDCEfficiency>85</DCDCEfficiency>
		</AMC>

		<AMC site="5">
			<PhysicalPort>5</PhysicalPort>
			<MaxCurrent>6.0</MaxCurrent>
			<PowerGoodTimeout>300</PowerGoodTimeout>
			<DCDCEfficiency>85</DCDCEfficiency>
		</AMC>

		<AMC site="6">
			<PhysicalPort>6</PhysicalPort>
			<MaxCurrent>6.0</MaxCurrent>
			<PowerGoodTimeout>300</PowerGoodTimeout>
			<DCDCEfficiency>85</DCDCEfficiency>
		</AMC>

		<AMC site="7">
			<PhysicalPort>7</PhysicalPort>
			<MaxCurrent>6.0</MaxCurrent>
			<PowerGoodTimeout>300</PowerGoodTimeout>
			<DCDCEfficiency>85</DCDCEfficiency>
		</AMC>

		<AMC site="8">
			<PhysicalPort>8</PhysicalPort>
			<MaxCurrent>6.0</MaxCurrent>
			<PowerGoodTimeout>300</PowerGoodTimeout>
			<DCDCEfficiency>85</DCDCEfficiency>
		</AMC>
		
	</AMCSlots>
	
	<iRTMSlot>
		<PhysicalPort>0</PhysicalPort>
		<MaxCurrent>6.0</MaxCurrent>
		<Address>0xea</Address>
		<PowerGoodTimeout>300</PowerGoodTimeout>
		<DCDCEfficiency>85</DCDCEfficiency>
	</iRTMSlot>
-->
	<SensorList>
	
		<Sensors type="MCP9801">
			
			<Sensor>
				<Name>Internal temp.</Name>
				
				<Type>Temperature</Type>
				<Units>degrees C</Units>
				
				<NominalReading>25</NominalReading>
				<NormalMaximum>60</NormalMaximum>
				<NormalMinimum>-10</NormalMinimum>
				
				<Point id="0" x="0" y="0" />
				<Point id="1" x="5" y="5" />
				
				<i2c_addr>0x090</i2c_addr>
				
				<Thresholds>
					<UpperNonRecovery>80</UpperNonRecovery>
					<UpperCritical>60</UpperCritical>
					<UpperNonCritical>40</UpperNonCritical>
					<LowerNonRecovery>-20</LowerNonRecovery>
					<LowerCritical>-10</LowerCritical>
					<LowerNonCritical>0</LowerNonCritical>
				</Thresholds>
				
			</Sensor>
			
		</Sensors>
			
	</SensorList>
</IPMC>
