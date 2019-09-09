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
    <ProductName>APOLLO-BLADE</ProductName>
    <ProductPN>PN00003</ProductPN>
    <ProductSN>0000003</ProductSN>
    <ProductVersion type="major">1</ProductVersion>	
    <ProductVersion type="minor">20</ProductVersion>			
    
    <MaxCurrent>20.0</MaxCurrent>
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
      <!-- ===========================
           **GPIO logic** are inverted 
           =========================== -->   

      <!-- no-shelf operation guard -->
      <!-- shut power sequence when not for shelf operation -->
      <step>PSQ_TEST_SIGNAL_JUMP_IFNOT_SET(USER_IO_17, 2)</step>
      <step>PSQ_FAIL</step>     

      <!-- IPMC Uart to mezz2 slot, where I can probe it -->
      <!-- Make sure UART ADDR [1:0] is "11" (inverted) -->        
      <step>PSQ_DISABLE_SIGNAL(USER_IO_5)</step> 
      <step>PSQ_DISABLE_SIGNAL(USER_IO_6)</step>

      <!-- turn on 12V (non-inverted) -->       
      <step>PSQ_ENABLE_SIGNAL(CFG_PAYLOAD_DCDC_EN_SIGNAL)</step> 

      <!-- Start Zynq power up sequence-->      
      <step>PSQ_DISABLE_SIGNAL(USER_IO_3)</step>

      <step>PSQ_DISABLE_SIGNAL(USER_IO_19)</step>

      <!-- let's wait 1s for power good to be received, fail if not -->
      <step>PSQ_SET_TIMER(0, 1000)</step> <!-- timer 0 -->
      <step>PSQ_JUMP_IFNOT_TIMEOUT(0, 4)</step>
      <step>PSQ_ENABLE_SIGNAL(USER_IO_3)</step> <!-- power off -->
      <step>PSQ_DISABLE_SIGNAL(CFG_PAYLOAD_DCDC_EN_SIGNAL)</step> <!-- power off -->
      <step>PSQ_FAIL</step> <!-- fail signaling -->
      <step>PSQ_TEST_SIGNAL_JUMP_IF_SET(USER_IO_13, -4)</step> <!-- eth_sw_pwr_good == 0? -->
      <step>PSQ_DISABLE_SIGNAL(USER_IO_20)</step>
      
      <!-- let's wait 10s for Zynq to wake up, fail if not -->
      <step>PSQ_SET_TIMER(1, 10000)</step> <!-- timer 1 -->
      <step>PSQ_JUMP_IFNOT_TIMEOUT(1, 4)</step>
      <step>PSQ_ENABLE_SIGNAL(USER_IO_3)</step>
      <step>PSQ_DISABLE_SIGNAL(CFG_PAYLOAD_DCDC_EN_SIGNAL)</step> <!-- power off -->
      <step>PSQ_FAIL</step>
      <step>PSQ_TEST_SIGNAL_JUMP_IF_SET(USER_IO_18, -4)</step> <!-- zynq_i2c_on? -->

      <step>PSQ_DISABLE_SIGNAL(USER_IO_21)</step>

      <!-- enable i2c mux -->
      <!--
      <step>PSQ_DISABLE_SIGNAL(USER_IO_9)</step>
      -->
      
      <!-- sucess!! -->
      <step>PSQ_END</step>     
    </PowerONSeq>
    
    <PowerOFFSeq>
      <!-- ===========================
           **GPIO logic** are inverted 
           =========================== -->   

      <!-- disable i2c mux -->
      <!--
      <step>PSQ_ENABLE_SIGNAL(USER_IO_9)</step>
      -->
      
      <step>PSQ_ENABLE_SIGNAL(USER_IO_3)</step>  <!-- Shutdow Zynq supplies-->   
      <step>PSQ_DISABLE_SIGNAL(CFG_PAYLOAD_DCDC_EN_SIGNAL)</step> <!-- turn off 12V-->     
      <step>PSQ_ENABLE_SIGNAL(USER_IO_19)</step>
      <step>PSQ_ENABLE_SIGNAL(USER_IO_20)</step>
      <step>PSQ_ENABLE_SIGNAL(USER_IO_21)</step>
      <step>PSQ_END</step>      
    </PowerOFFSeq>
    
  </PowerManagement>
  
  <LANConfig>
    
    <MACAddr>${mac_address}</MACAddr>
    <NetMask>255.255.0.0</NetMask>
    <GatewayIP>192.168.20.1</GatewayIP>
    
    <!-- <EnableDHCP /> -->
    
    <IPAddrList> <!-- Default IP Addresses (used if DHCP is not active) -->
      <IPAddr slot_addr="default">${ip_address}</IPAddr>
      <IPAddr slot_addr="0x41">192.168.20.20</IPAddr>
      <IPAddr slot_addr="0x42">192.168.20.21</IPAddr>
      <IPAddr slot_addr="0x43">192.168.20.22</IPAddr>
      <IPAddr slot_addr="0x44">192.168.20.23</IPAddr>
      <IPAddr slot_addr="0x45">192.168.20.24</IPAddr>
      <IPAddr slot_addr="0x46">192.168.20.25</IPAddr>
      <IPAddr slot_addr="0x47">192.168.20.26</IPAddr>
      <IPAddr slot_addr="0x48">192.168.20.27</IPAddr>
      <IPAddr slot_addr="0x49">192.168.20.28</IPAddr>
      <IPAddr slot_addr="0x4a">192.168.20.29</IPAddr>
      <IPAddr slot_addr="0x4b">192.168.20.30</IPAddr>
      <IPAddr slot_addr="0x4c">192.168.20.31</IPAddr>
      <IPAddr slot_addr="0x4d">192.168.20.32</IPAddr>
      <IPAddr slot_addr="0x4e">192.168.20.33</IPAddr>
    </IPAddrList>
    
  </LANConfig>
  
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

    <Sensors type="raw" global_define="CFG_SENSOR_TCN75" function_name="SENSOR_TCN75" rawType="TCN75"> 
			 
      <Sensor> 
        <Name>U34 Temp</Name> 
        
        <Type>Temperature</Type> 
        <Units>degrees C</Units> 
        
        <NominalReading>25</NominalReading> 
        <NormalMaximum>50</NormalMaximum> 
        <NormalMinimum>0</NormalMinimum> 
        
        <Point id="0" x="0" y="0" /> 
        <Point id="1" x="7" y="7" /> 
        
        <Thresholds> 
          <UpperNonRecovery>50</UpperNonRecovery> 
          <UpperCritical>43</UpperCritical> 
          <UpperNonCritical>38</UpperNonCritical> 
          <LowerNonRecovery>0</LowerNonRecovery> 
          <LowerCritical>0</LowerCritical> 
          <LowerNonCritical>0</LowerNonCritical> 
        </Thresholds> 
        
        <Params> 
          <p type="record_id">100</p>  <!-- mandatory --> 
          <p type="user">34</p> <!--unsigned char id-->
          <p type="user">UCGH | UNRGH</p> 
        </Params> 
        
        <AssertEvMask>0x0A80</AssertEvMask> 
        <DeassertEvMask>0x7A80</DeassertEvMask> 
        <DiscreteRdMask>0x3838</DiscreteRdMask> 
        <AnalogDataFmt>UNSIGNED</AnalogDataFmt> 
        <PosHysteresis>0</PosHysteresis> 
        <NegHysteresis>0</NegHysteresis> 
        <MaxReading>80</MaxReading> 
        <MinReading>0</MinReading> 
        
      </Sensor> 
      
      <Sensor> 
        <Name>U35 Temp</Name> 
        
        <Type>Temperature</Type> 
        <Units>degrees C</Units> 
        
        <NominalReading>25</NominalReading> 
        <NormalMaximum>50</NormalMaximum> 
        <NormalMinimum>0</NormalMinimum> 
        
        <Point id="0" x="0" y="0" /> 
        <Point id="1" x="7" y="7" /> 
        
        <Thresholds> 
          <UpperNonRecovery>50</UpperNonRecovery> 
          <UpperCritical>43</UpperCritical> 
          <UpperNonCritical>38</UpperNonCritical> 
          <LowerNonRecovery>0</LowerNonRecovery> 
          <LowerCritical>0</LowerCritical> 
          <LowerNonCritical>0</LowerNonCritical> 
        </Thresholds> 
        
        <Params> 
          <p type="record_id">101</p>  <!-- mandatory --> 
          <p type="user">35</p> <!--unsigned char id-->
          <p type="user">UCGH | UNRGH</p> 
        </Params> 
        
        <AssertEvMask>0x0A80</AssertEvMask> 
        <DeassertEvMask>0x7A80</DeassertEvMask> 
        <DiscreteRdMask>0x3838</DiscreteRdMask> 
        <AnalogDataFmt>UNSIGNED</AnalogDataFmt> 
        <PosHysteresis>0</PosHysteresis> 
        <NegHysteresis>0</NegHysteresis> 
        <MaxReading>80</MaxReading> 
        <MinReading>0</MinReading> 
      </Sensor> 
      <Sensor> 
        <Name>U36 Temp</Name> 
        
        <Type>Temperature</Type> 
        <Units>degrees C</Units> 
        
        <NominalReading>25</NominalReading> 
        <NormalMaximum>50</NormalMaximum> 
        <NormalMinimum>0</NormalMinimum> 
        
        <Point id="0" x="0" y="0" /> 
        <Point id="1" x="7" y="7" /> 
        
        <Thresholds> 
          <UpperNonRecovery>50</UpperNonRecovery> 
          <UpperCritical>43</UpperCritical> 
          <UpperNonCritical>38</UpperNonCritical> 
          <LowerNonRecovery>0</LowerNonRecovery> 
          <LowerCritical>0</LowerCritical> 
          <LowerNonCritical>0</LowerNonCritical> 
        </Thresholds> 
        
        <Params> 
          <p type="record_id">102</p>  <!-- mandatory --> 
          <p type="user">36</p> <!--unsigned char id-->
          <p type="user">UCGH | UNRGH</p> 
        </Params> 
        
        <AssertEvMask>0x0A80</AssertEvMask> 
        <DeassertEvMask>0x7A80</DeassertEvMask> 
        <DiscreteRdMask>0x3838</DiscreteRdMask> 
        <AnalogDataFmt>UNSIGNED</AnalogDataFmt> 
        <PosHysteresis>0</PosHysteresis> 
        <NegHysteresis>0</NegHysteresis> 
        <MaxReading>80</MaxReading> 
        <MinReading>0</MinReading>     
      </Sensor> 
    </Sensors>
    
  </SensorList>
</IPMC>
