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
    <ProductPN>${pn}</ProductPN>
    <ProductSN>${sn}</ProductSN>
    <ProductVersion type="major">${major}</ProductVersion>	
    <ProductVersion type="minor">${minor}</ProductVersion>			
    
    <MaxCurrent>20.0</MaxCurrent>
    <MaxInternalCurrent>1.0</MaxInternalCurrent>
    
    <!-- Hardware -->
    <HandleSwitch active="LOW" inactive="HIGH" />
    
    <!-- <ResetOnWrongHAEn /> -->
    <!-- <PowerMonitoringEn /> -->
    <!-- <AlertMonitoringEn />-->

    ${serial}
    
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

      <!-- init -->
      <step>PSQ_ENABLE_SIGNAL(IPM_IO_0)</step> <!-- delay -->
      <step>PSQ_ENABLE_SIGNAL(IPM_IO_1)</step> <!-- delay -->
      <step>PSQ_ENABLE_SIGNAL(IPM_IO_2)</step> <!-- delay -->
      <step>PSQ_ENABLE_SIGNAL(IPM_IO_3)</step> <!-- delay -->
      <step>PSQ_ENABLE_SIGNAL(IPM_IO_4)</step> <!-- delay -->
      <step>PSQ_ENABLE_SIGNAL(IPM_IO_5)</step> <!-- delay -->
      <step>PSQ_ENABLE_SIGNAL(IPM_IO_10)</step> <!-- off req -->
      
      <!-- debug_0: tracking power up -->
      <step>PSQ_DISABLE_SIGNAL(USER_IO_19)</step>

      <!-- Route UART to void -->
      <!-- Make sure UART ADDR 1 downto 0 is "01" -->        
      <step>PSQ_DISABLE_SIGNAL(USER_IO_5)</step> <!-- Set ADDR0 to 1-->
      <step>PSQ_ENABLE_SIGNAL(USER_IO_6)</step>  <!-- Set ADDR1 to 0-->

      <!-- turn on 12V (non-inverted) -->
      <step>PSQ_ENABLE_SIGNAL(CFG_PAYLOAD_DCDC_EN_SIGNAL)</step> 

      <!-- Start Zynq power up sequence (inverted) -->      
      <step>PSQ_DISABLE_SIGNAL(USER_IO_3)</step>

      <!-- let's wait 1s for power good to be received, fail if not -->
      <step>PSQ_SET_TIMER(0, 1000)</step> <!-- timer 0 -->
      <step>PSQ_JUMP_IFNOT_TIMEOUT(0, 4)</step>
      <step>PSQ_ENABLE_SIGNAL(USER_IO_3)</step> <!-- power off -->
      <step>PSQ_DISABLE_SIGNAL(CFG_PAYLOAD_DCDC_EN_SIGNAL)</step> <!-- power off -->
      <step>PSQ_FAIL</step> <!-- fail signaling -->
      <step>PSQ_TEST_SIGNAL_JUMP_IF_SET(USER_IO_13, -4)</step> <!-- eth_sw_pwr_good == 0? -->

      <!-- Route UART to ${uart_target} -->
      ${uart}

      <!-- let's wait some sec for Zynq to wake up, fail if not -->
      <step>PSQ_SET_TIMER(1, 10000)</step> <!-- timer 1 -->
      <step>PSQ_JUMP_IFNOT_TIMEOUT(1, 22)</step>

      <!-- timeout 1-->
      <step>PSQ_TEST_SIGNAL_JUMP_IFNOT_SET(IPM_IO_0, 3)</step>
      <step>PSQ_DISABLE_SIGNAL(IPM_IO_0)</step>
      <step>PSQ_JUMP(-4)</step>

      <!-- timeout 2 -->
      <step>PSQ_TEST_SIGNAL_JUMP_IFNOT_SET(IPM_IO_1, 3)</step>
      <step>PSQ_DISABLE_SIGNAL(IPM_IO_1)</step>
      <step>PSQ_JUMP(-3)</step>

      <!-- timeout 3 -->
      <step>PSQ_TEST_SIGNAL_JUMP_IFNOT_SET(IPM_IO_2, 3)</step>
      <step>PSQ_DISABLE_SIGNAL(IPM_IO_2)</step>
      <step>PSQ_JUMP(-3)</step>

      <!-- timeout 4 -->
      <step>PSQ_TEST_SIGNAL_JUMP_IFNOT_SET(IPM_IO_3, 3)</step>
      <step>PSQ_DISABLE_SIGNAL(IPM_IO_3)</step>
      <step>PSQ_JUMP(-3)</step>

      <!-- timeout 5 -->
      <step>PSQ_TEST_SIGNAL_JUMP_IFNOT_SET(IPM_IO_4, 3)</step>
      <step>PSQ_DISABLE_SIGNAL(IPM_IO_4)</step>
      <step>PSQ_JUMP(-3)</step>

      <!-- timeout 6 -->
      <step>PSQ_TEST_SIGNAL_JUMP_IFNOT_SET(IPM_IO_5, 3)</step>
      <step>PSQ_DISABLE_SIGNAL(IPM_IO_5)</step>
      <step>PSQ_JUMP(-3)</step>

      <!-- failed... turn everything off -->
      <step>PSQ_ENABLE_SIGNAL(USER_IO_3)</step>
      <step>PSQ_DISABLE_SIGNAL(CFG_PAYLOAD_DCDC_EN_SIGNAL)</step> <!-- power off -->
      <step>PSQ_FAIL</step>

      <step>PSQ_TEST_SIGNAL_JUMP_IF_SET(USER_IO_18, -22)</step> <!-- zynq_i2c_on? -->

      <!-- debug_0: tracking power up -->
      <step>PSQ_ENABLE_SIGNAL(USER_IO_19)</step>

      <!-- sucess!! -->
      <step>PSQ_END</step>     
      
    </PowerONSeq>
    
    <PowerOFFSeq>
      <!-- ===========================
           **GPIO logic** are inverted 
           =========================== -->   

      <!-- debug_1: tracking power off -->
      <step>PSQ_DISABLE_SIGNAL(USER_IO_20)</step>

      <!-- init counter flags -->
      <step>PSQ_ENABLE_SIGNAL(IPM_IO_0)</step>
      <step>PSQ_ENABLE_SIGNAL(IPM_IO_1)</step>
      <step>PSQ_ENABLE_SIGNAL(IPM_IO_2)</step>
      <step>PSQ_ENABLE_SIGNAL(IPM_IO_3)</step>
      <step>PSQ_ENABLE_SIGNAL(IPM_IO_4)</step>
      <step>PSQ_ENABLE_SIGNAL(IPM_IO_5)</step>

      <!-- request CM shutdown -->
      <step>PSQ_DISABLE_SIGNAL(IPM_IO_10)</step>

      <!-- let's wait some secs for CM to shut down, ignore if timeout -->
      <step>PSQ_SET_TIMER(1, 10000)</step> <!-- timer 1 -->
      <step>PSQ_JUMP_IFNOT_TIMEOUT(1, 19)</step>  <!-- 0 -20 -->

      <!-- timeout 1-->
      <step>PSQ_TEST_SIGNAL_JUMP_IFNOT_SET(IPM_IO_0, 3)</step> <!-- 1 -19 -->
      <step>PSQ_DISABLE_SIGNAL(IPM_IO_0)</step> <!-- 2 -18 -->
      <step>PSQ_JUMP(-4)</step> <!-- 3 -17 -->

      <!-- timeout 2 -->
      <step>PSQ_TEST_SIGNAL_JUMP_IFNOT_SET(IPM_IO_1, 3)</step> <!-- 4 -16 -->
      <step>PSQ_DISABLE_SIGNAL(IPM_IO_1)</step> <!-- 5 -15 -->
      <step>PSQ_JUMP(-3)</step> <!-- 6 -14 -->

      <!-- timeout 3 -->
      <step>PSQ_TEST_SIGNAL_JUMP_IFNOT_SET(IPM_IO_2, 3)</step> <!-- 7 -13 -->
      <step>PSQ_DISABLE_SIGNAL(IPM_IO_2)</step> <!-- 8 -12 -->
      <step>PSQ_JUMP(-3)</step> <!-- 9 -11 -->

      <!-- timeout 4 -->
      <step>PSQ_TEST_SIGNAL_JUMP_IFNOT_SET(IPM_IO_3, 3)</step> <!-- 10 -10 -->
      <step>PSQ_DISABLE_SIGNAL(IPM_IO_3)</step> <!-- 11 -9 -->
      <step>PSQ_JUMP(-3)</step> <!-- 12 -8 -->

      <!-- timeout 5 -->
      <step>PSQ_TEST_SIGNAL_JUMP_IFNOT_SET(IPM_IO_4, 3)</step> <!-- 13 -7 -->
      <step>PSQ_DISABLE_SIGNAL(IPM_IO_4)</step> <!-- 14 -6 -->
      <step>PSQ_JUMP(-3)</step> <!-- 15 -5 -->

      <!-- timeout 6 -->
      <step>PSQ_TEST_SIGNAL_JUMP_IFNOT_SET(IPM_IO_5, 5)</step> <!-- 16 -4 -->
      <step>PSQ_DISABLE_SIGNAL(IPM_IO_5)</step> <!-- 17 -3 -->
      <step>PSQ_JUMP(-3)</step> <!-- 18 -2 -->

      <!-- cm off? -->
      <step>PSQ_TEST_SIGNAL_JUMP_IF_SET(IPM_IO_11, -19)</step> <!-- 20 0 -->

      <!-- Shutdow Zynq supplies-->
      <step>PSQ_ENABLE_SIGNAL(USER_IO_3)</step>  
      <!-- turn off 12V-->
      <step>PSQ_DISABLE_SIGNAL(CFG_PAYLOAD_DCDC_EN_SIGNAL)</step> 

      <!-- remove CM shutdown request-->
      <step>PSQ_ENABLE_SIGNAL(IPM_IO_10)</step>

      <!-- Route UART to void -->
      <!-- Make sure UART ADDR 1 downto 0 is "01" -->        
      <step>PSQ_DISABLE_SIGNAL(USER_IO_5)</step> <!-- Set ADDR0 to 1-->
      <step>PSQ_ENABLE_SIGNAL(USER_IO_6)</step>  <!-- Set ADDR1 to 0-->

      <!-- debug_1: tracking power off -->
      <step>PSQ_ENABLE_SIGNAL(USER_IO_20)</step>

      <step>PSQ_END</step>      

    </PowerOFFSeq>
    
  </PowerManagement>
  
  <LANConfig>

    <MACAddr>${mac_addr}</MACAddr>
    <NetMask>${netmask}</NetMask>
    <GatewayIP>${gateway}</GatewayIP>

    ${flashed_mac_addr}

    ${dhcp}
    
    <IPAddrList> <!-- Default IP Addresses (used if DHCP is not active) -->
      <IPAddr slot_addr="default">${ip_addr}</IPAddr>
      <IPAddr slot_addr="0x41">${slot_01_ip_addr}</IPAddr>
      <IPAddr slot_addr="0x42">${slot_02_ip_addr}</IPAddr>
      <IPAddr slot_addr="0x43">${slot_03_ip_addr}</IPAddr>
      <IPAddr slot_addr="0x44">${slot_04_ip_addr}</IPAddr>
      <IPAddr slot_addr="0x45">${slot_05_ip_addr}</IPAddr>
      <IPAddr slot_addr="0x46">${slot_06_ip_addr}</IPAddr>
      <IPAddr slot_addr="0x47">${slot_07_ip_addr}</IPAddr>
      <IPAddr slot_addr="0x48">${slot_08_ip_addr}</IPAddr>
      <IPAddr slot_addr="0x49">${slot_09_ip_addr}</IPAddr>
      <IPAddr slot_addr="0x4a">${slot_10_ip_addr}</IPAddr>
      <IPAddr slot_addr="0x4b">${slot_11_ip_addr}</IPAddr>
      <IPAddr slot_addr="0x4c">${slot_12_ip_addr}</IPAddr>
      <IPAddr slot_addr="0x4d">${slot_13_ip_addr}</IPAddr>
      <IPAddr slot_addr="0x4e">${slot_14_ip_addr}</IPAddr>
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
        <Name>SM Top Temp</Name> 
        
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
          <p type="record_id"></p>  <!-- mandatory --> 
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
        <Name>SM Bottom Temp</Name> 
        
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
          <p type="record_id"></p>  <!-- mandatory --> 
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
        <Name>SM Center Temp</Name> 
        
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
          <p type="record_id"></p>  <!-- mandatory --> 
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

    <Sensors type="raw" global_define="CFG_SENSOR_ZYNQ" function_name="SENSOR_ZYNQ" rawType="ZYNQ"> 

      <Sensor> 
        <!-- Slave 0x61, reg 0 is the CM uC temp in degrees C --> 
        <Name>ZP CM MCU Temp</Name> 
        
        <Type>Temperature</Type> 
        <Units>degrees C</Units> 
        
        <NominalReading>25</NominalReading> 
        <NormalMaximum>50</NormalMaximum> 
        <NormalMinimum>0</NormalMinimum> 
        
        <Point id="0" x="0" y="0" /> 
        <Point id="1" x="7" y="7" /> 
        
        <Thresholds> 
          <UpperNonRecovery>55</UpperNonRecovery> 
          <UpperCritical>50</UpperCritical> 
          <UpperNonCritical>44</UpperNonCritical> 
          <LowerNonRecovery>0</LowerNonRecovery> 
          <LowerCritical>0</LowerCritical> 
          <LowerNonCritical>0</LowerNonCritical> 
        </Thresholds> 
        
        <Params> 
          <p type="record_id"></p>  <!-- mandatory --> 
          <p type="user">0x61</p> <!--unsigned char addr-->
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
        <!-- slave 0x63 reg 0 is the max CM FPGA temp in degrees C -->
        <Name>ZP CM FPGA Temp</Name> 
        
        <Type>Temperature</Type> 
        <Units>degrees C</Units> 
        
        <NominalReading>25</NominalReading> 
        <NormalMaximum>50</NormalMaximum> 
        <NormalMinimum>0</NormalMinimum> 
        
        <Point id="0" x="0" y="0" /> 
        <Point id="1" x="7" y="7" /> 
        
        <Thresholds> 
          <UpperNonRecovery>80</UpperNonRecovery> 
          <UpperCritical>65</UpperCritical> 
          <UpperNonCritical>50</UpperNonCritical> 
          <LowerNonRecovery>0</LowerNonRecovery> 
          <LowerCritical>0</LowerCritical> 
          <LowerNonCritical>0</LowerNonCritical> 
        </Thresholds> 
        
        <Params> 
          <p type="record_id"></p>  <!-- mandatory --> 
          <p type="user">0x63</p> <!--unsigned char addr-->
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
        <!-- slave 0x62 reg 0 is max Firefly temp in degrees C -->
        <Name>ZP CM FF Temp</Name> 
        
        <Type>Temperature</Type> 
        <Units>degrees C</Units> 
        
        <NominalReading>25</NominalReading> 
        <NormalMaximum>50</NormalMaximum> 
        <NormalMinimum>0</NormalMinimum> 
        
        <Point id="0" x="0" y="0" /> 
        <Point id="1" x="7" y="7" /> 
        
        <Thresholds> 
          <UpperNonRecovery>55</UpperNonRecovery> 
          <UpperCritical>50</UpperCritical> 
          <UpperNonCritical>44</UpperNonCritical> 
          <LowerNonRecovery>0</LowerNonRecovery> 
          <LowerCritical>0</LowerCritical> 
          <LowerNonCritical>0</LowerNonCritical> 
        </Thresholds> 
        
        <Params> 
          <p type="record_id"></p>  <!-- mandatory --> 
          <p type="user">0x62</p> <!--unsigned char addr-->
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
        <!-- slave 0x64 reg 0 is the max CM regulator temp in degrees C -->
        <Name>ZP CM Reg Temp</Name> 
        
        <Type>Temperature</Type> 
        <Units>degrees C</Units> 
        
        <NominalReading>25</NominalReading> 
        <NormalMaximum>50</NormalMaximum> 
        <NormalMinimum>0</NormalMinimum> 
        
        <Point id="0" x="0" y="0" /> 
        <Point id="1" x="7" y="7" /> 
        
        <Thresholds> 
          <UpperNonRecovery>65</UpperNonRecovery> 
          <UpperCritical>55</UpperCritical> 
          <UpperNonCritical>45</UpperNonCritical> 
          <LowerNonRecovery>0</LowerNonRecovery> 
          <LowerCritical>0</LowerCritical> 
          <LowerNonCritical>0</LowerNonCritical> 
        </Thresholds> 
        
        <Params> 
          <p type="record_id"></p>  <!-- mandatory --> 
          <p type="user">0x64</p> <!--unsigned char addr-->
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

    <Sensors type="raw" global_define="CFG_SENSOR_MCU" function_name="SENSOR_MCU" rawType="MCU"> 

      <Sensor> 
        <!-- 0x10    CM_MCU_TEMP     uint8, LSB is 1C -->
        <Name>CM MCU Temp</Name> 
        
        <Type>Temperature</Type> 
        <Units>degrees C</Units> 
        
        <NominalReading>25</NominalReading> 
        <NormalMaximum>50</NormalMaximum> 
        <NormalMinimum>0</NormalMinimum> 
        
        <Point id="0" x="0" y="0" /> 
        <Point id="1" x="7" y="7" /> 
        
        <Thresholds> 
          <UpperNonRecovery>55</UpperNonRecovery> 
          <UpperCritical>50</UpperCritical> 
          <UpperNonCritical>44</UpperNonCritical> 
          <LowerNonRecovery>0</LowerNonRecovery> 
          <LowerCritical>0</LowerCritical> 
          <LowerNonCritical>0</LowerNonCritical> 
        </Thresholds> 
        
        <Params> 
          <p type="record_id"></p>  <!-- mandatory --> 
          <p type="user">0x40</p> <!--unsigned char addr-->
          <p type="user">0x10</p> <!--unsigned char addr-->
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
        <!-- 0x12    CM_FPGA_VU_TEMP uint8, LSB is 1C -->
        <Name>CM FPGA VU Temp</Name> 
        
        <Type>Temperature</Type> 
        <Units>degrees C</Units> 
        
        <NominalReading>25</NominalReading> 
        <NormalMaximum>50</NormalMaximum> 
        <NormalMinimum>0</NormalMinimum> 
        
        <Point id="0" x="0" y="0" /> 
        <Point id="1" x="7" y="7" /> 
        
        <Thresholds> 
          <UpperNonRecovery>80</UpperNonRecovery> 
          <UpperCritical>65</UpperCritical> 
          <UpperNonCritical>50</UpperNonCritical> 
          <LowerNonRecovery>0</LowerNonRecovery> 
          <LowerCritical>0</LowerCritical> 
          <LowerNonCritical>0</LowerNonCritical> 
        </Thresholds> 
        
        <Params> 
          <p type="record_id"></p>  <!-- mandatory --> 
          <p type="user">0x40</p> <!--unsigned char addr-->
          <p type="user">0x12</p> <!--unsigned char addr-->
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
        <!-- 0x14    CM_FPGA_KU_TEMP uint8, LSB is 1C -->
        <Name>CM FPGA KU Temp</Name> 
        
        <Type>Temperature</Type> 
        <Units>degrees C</Units> 
        
        <NominalReading>25</NominalReading> 
        <NormalMaximum>50</NormalMaximum> 
        <NormalMinimum>0</NormalMinimum> 
        
        <Point id="0" x="0" y="0" /> 
        <Point id="1" x="7" y="7" /> 
        
        <Thresholds> 
          <UpperNonRecovery>80</UpperNonRecovery> 
          <UpperCritical>65</UpperCritical> 
          <UpperNonCritical>50</UpperNonCritical> 
          <LowerNonRecovery>0</LowerNonRecovery> 
          <LowerCritical>0</LowerCritical> 
          <LowerNonCritical>0</LowerNonCritical> 
        </Thresholds> 
        
        <Params> 
          <p type="record_id"></p>  <!-- mandatory --> 
          <p type="user">0x40</p> <!--unsigned char addr-->
          <p type="user">0x14</p> <!--unsigned char addr-->
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
        <!-- 0x16    CM_FF_TEMP      uint8, LSB is 1C -->
        <Name>CM Max FF Temp</Name> 
        
        <Type>Temperature</Type> 
        <Units>degrees C</Units> 
        
        <NominalReading>25</NominalReading> 
        <NormalMaximum>+126</NormalMaximum> 
        <NormalMinimum>-126</NormalMinimum> 
        
        <Point id="0" x="0" y="0" /> 
        <Point id="1" x="7" y="7" /> 
        
        <Thresholds> 
          <UpperNonRecovery>55</UpperNonRecovery> 
          <UpperCritical>50</UpperCritical> 
          <UpperNonCritical>44</UpperNonCritical> 
          <LowerNonRecovery>0</LowerNonRecovery> 
          <LowerCritical>0</LowerCritical> 
          <LowerNonCritical>0</LowerNonCritical> 
        </Thresholds> 

        <Params> 
          <p type="record_id"></p>  <!-- mandatory --> 
          <p type="user">0x40</p> <!--unsigned char addr-->
          <p type="user">0x16</p> <!--unsigned char addr-->
          <p type="user">UCGH | UNRGH</p> 
        </Params> 
        
        <AssertEvMask>0x0A80</AssertEvMask> 
        <DeassertEvMask>0x7A80</DeassertEvMask> 
        <DiscreteRdMask>0x3838</DiscreteRdMask> 
        <AnalogDataFmt>2S_COMPL</AnalogDataFmt> 
        <PosHysteresis>0</PosHysteresis> 
        <NegHysteresis>0</NegHysteresis> 
        <MaxReading>80</MaxReading> 
        <MinReading>0</MinReading> 
        
      </Sensor>

      <Sensor> 
        <!-- 0x18    CM_REG_TEMP     uint8, LSB is 1C -->
        <Name>CM Max Reg Temp</Name> 
        
        <Type>Temperature</Type> 
        <Units>degrees C</Units> 
        
        <NominalReading>25</NominalReading> 
        <NormalMaximum>50</NormalMaximum> 
        <NormalMinimum>0</NormalMinimum> 
        
        <Point id="0" x="0" y="0" /> 
        <Point id="1" x="7" y="7" /> 
        
        <Thresholds> 
          <UpperNonRecovery>65</UpperNonRecovery> 
          <UpperCritical>55</UpperCritical> 
          <UpperNonCritical>45</UpperNonCritical> 
          <LowerNonRecovery>0</LowerNonRecovery> 
          <LowerCritical>0</LowerCritical> 
          <LowerNonCritical>0</LowerNonCritical> 
        </Thresholds> 
        
        <Params> 
          <p type="record_id"></p>  <!-- mandatory --> 
          <p type="user">0x40</p> <!--unsigned char addr-->
          <p type="user">0x18</p> <!--unsigned char addr-->
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

    <Sensors type="raw" global_define="CFG_SENSOR_PIM400KZ" function_name="SENSOR_PIM400KZ" rawType="PIM400KZ"> 

      <Sensor> 

        <Name>PIM400KZ Temp</Name> 
        
        <Type>Temperature</Type> 
        <Units>degrees C</Units> 
        
        <NominalReading>25</NominalReading> 
        <NormalMaximum>50</NormalMaximum> 
        <NormalMinimum>0</NormalMinimum> 
        
        <Point id="0" x="29" y="6.879" /> 
        <Point id="1" x="50" y="48.05" /> 
        
        <Thresholds> 
          <UpperNonRecovery>85</UpperNonRecovery> 
          <UpperCritical>75</UpperCritical> 
          <UpperNonCritical>65</UpperNonCritical> 
          <LowerNonRecovery>0</LowerNonRecovery> 
          <LowerCritical>0</LowerCritical> 
          <LowerNonCritical>0</LowerNonCritical> 
        </Thresholds> 
        
        <Params> 
          <p type="record_id"></p>  <!-- mandatory --> 
          <p type="user">0</p> <!--unsigned char addr-->
          <p type="user">UCGH | UNRGH</p> 
        </Params> 
        
        <AssertEvMask>0x0A80</AssertEvMask> 
        <DeassertEvMask>0x7A80</DeassertEvMask> 
        <DiscreteRdMask>0x3838</DiscreteRdMask> 
        <AnalogDataFmt>UNSIGNED</AnalogDataFmt> 
        <PosHysteresis>0</PosHysteresis> 
        <NegHysteresis>0</NegHysteresis> 
        <MaxReading>90</MaxReading> 
        <MinReading>0</MinReading> 
        
      </Sensor>    
      
      <Sensor> 

        <Name>PIM400KZ Current</Name> 
        
        <Type>Current</Type> 
        <Units>Amps</Units> 
        
        <NominalReading>5</NominalReading> 
        <NormalMaximum>8</NormalMaximum> 
        <NormalMinimum>0</NormalMinimum> 
        
        <Point id="0" x="0" y="0" /> 
        <Point id="1" x="50" y="4.7" /> 
        
        <Thresholds> 
          <UpperNonRecovery>8.5</UpperNonRecovery> 
          <UpperCritical>8</UpperCritical> 
          <UpperNonCritical>7.5</UpperNonCritical> 
          <LowerNonRecovery>0</LowerNonRecovery> 
          <LowerCritical>0</LowerCritical> 
          <LowerNonCritical>0</LowerNonCritical> 
        </Thresholds> 
        
        <Params> 
          <p type="record_id"></p>  <!-- mandatory --> 
          <p type="user">1</p> <!--unsigned char addr-->
          <p type="user">UCGH | UNRGH</p> 
        </Params> 
        
        <AssertEvMask>0x0A80</AssertEvMask> 
        <DeassertEvMask>0x7A80</DeassertEvMask> 
        <DiscreteRdMask>0x3838</DiscreteRdMask> 
        <AnalogDataFmt>UNSIGNED</AnalogDataFmt> 
        <PosHysteresis>0</PosHysteresis> 
        <NegHysteresis>0</NegHysteresis> 
        <MaxReading>10</MaxReading> 
        <MinReading>0</MinReading> 
        
      </Sensor>    
      
    </Sensors>

  </SensorList>
</IPMC>
