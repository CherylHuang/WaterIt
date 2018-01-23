<GameFile>
  <PropertyGroup Name="MainMenu" Type="Scene" ID="ffe6d0d5-c028-4831-acad-bec95143c22f" Version="3.10.0.0" />
  <Content ctype="GameProjectContent">
    <Content>
      <Animation Duration="0" Speed="1.0000" />
      <ObjectData Name="Scene" Tag="23" ctype="GameNodeObjectData">
        <Size X="1280.0000" Y="720.0000" />
        <Children>
          <AbstractNodeData Name="bg" CanEdit="False" ActionTag="-1167602264" Tag="25" IconVisible="False" LeftMargin="-1.9987" RightMargin="-2.0012" TopMargin="-121.9999" BottomMargin="-122.0001" ctype="SpriteObjectData">
            <Size X="1284.0000" Y="964.0000" />
            <AnchorPoint ScaleX="0.5000" ScaleY="0.5000" />
            <Position X="640.0013" Y="359.9999" />
            <Scale ScaleX="1.0000" ScaleY="0.7533" />
            <CColor A="255" R="255" G="255" B="255" />
            <PrePosition X="0.5000" Y="0.5000" />
            <PreSize X="1.0031" Y="1.3389" />
            <FileData Type="Normal" Path="background.png" Plist="" />
            <BlendFunc Src="1" Dst="771" />
          </AbstractNodeData>
          <AbstractNodeData Name="game_name_tx" ActionTag="1149869131" Tag="26" IconVisible="False" LeftMargin="485.0000" RightMargin="485.0000" TopMargin="51.3906" BottomMargin="578.6094" FontSize="80" LabelText="Water it !" OutlineSize="3" OutlineEnabled="True" ShadowOffsetX="2.0000" ShadowOffsetY="-2.0000" ShadowEnabled="True" ctype="TextObjectData">
            <Size X="310.0000" Y="90.0000" />
            <AnchorPoint ScaleX="0.5000" ScaleY="0.5000" />
            <Position X="640.0000" Y="623.6094" />
            <Scale ScaleX="1.0000" ScaleY="1.0000" />
            <CColor A="255" R="139" G="68" B="20" />
            <PrePosition X="0.5000" Y="0.8661" />
            <PreSize X="0.2422" Y="0.1250" />
            <FontResource Type="Normal" Path="Marker Felt.ttf" Plist="" />
            <OutlineColor A="255" R="255" G="255" B="255" />
            <ShadowColor A="255" R="110" G="110" B="110" />
          </AbstractNodeData>
          <AbstractNodeData Name="wall_1" ActionTag="-1243456457" Tag="36" IconVisible="False" LeftMargin="480.8349" RightMargin="667.1651" TopMargin="394.4998" BottomMargin="228.5002" ctype="SpriteObjectData">
            <Size X="132.0000" Y="97.0000" />
            <AnchorPoint ScaleX="0.5000" ScaleY="0.5000" />
            <Position X="546.8349" Y="277.0002" />
            <Scale ScaleX="0.7338" ScaleY="0.7338" />
            <CColor A="255" R="255" G="255" B="255" />
            <PrePosition X="0.4272" Y="0.3847" />
            <PreSize X="0.1031" Y="0.1347" />
            <FileData Type="PlistSubImage" Path="14.png" Plist="box2d_bg_pic.plist" />
            <BlendFunc Src="1" Dst="771" />
          </AbstractNodeData>
          <AbstractNodeData Name="wall_2" ActionTag="-1510357595" Tag="37" IconVisible="False" LeftMargin="574.0005" RightMargin="573.9995" TopMargin="394.4998" BottomMargin="228.5002" ctype="SpriteObjectData">
            <Size X="132.0000" Y="97.0000" />
            <AnchorPoint ScaleX="0.5000" ScaleY="0.5000" />
            <Position X="640.0005" Y="277.0002" />
            <Scale ScaleX="0.7338" ScaleY="0.7338" />
            <CColor A="255" R="255" G="255" B="255" />
            <PrePosition X="0.5000" Y="0.3847" />
            <PreSize X="0.1031" Y="0.1347" />
            <FileData Type="PlistSubImage" Path="15.png" Plist="box2d_bg_pic.plist" />
            <BlendFunc Src="1" Dst="771" />
          </AbstractNodeData>
          <AbstractNodeData Name="wall_3" ActionTag="1641741433" Tag="38" IconVisible="False" LeftMargin="666.8359" RightMargin="481.1641" TopMargin="394.4998" BottomMargin="228.5002" ctype="SpriteObjectData">
            <Size X="132.0000" Y="97.0000" />
            <AnchorPoint ScaleX="0.5000" ScaleY="0.5000" />
            <Position X="732.8359" Y="277.0002" />
            <Scale ScaleX="0.7338" ScaleY="0.7338" />
            <CColor A="255" R="255" G="255" B="255" />
            <PrePosition X="0.5725" Y="0.3847" />
            <PreSize X="0.1031" Y="0.1347" />
            <FileData Type="PlistSubImage" Path="16.png" Plist="box2d_bg_pic.plist" />
            <BlendFunc Src="1" Dst="771" />
          </AbstractNodeData>
          <AbstractNodeData Name="Bush_dry" ActionTag="-1459962015" Tag="39" IconVisible="False" LeftMargin="572.5005" RightMargin="572.4995" TopMargin="334.5907" BottomMargin="309.4093" ctype="SpriteObjectData">
            <Size X="135.0000" Y="76.0000" />
            <AnchorPoint ScaleX="0.5000" ScaleY="0.5000" />
            <Position X="640.0005" Y="347.4093" />
            <Scale ScaleX="1.0000" ScaleY="1.0000" />
            <CColor A="255" R="255" G="255" B="255" />
            <PrePosition X="0.5000" Y="0.4825" />
            <PreSize X="0.1055" Y="0.1056" />
            <FileData Type="PlistSubImage" Path="Bush1.png" Plist="box2d_pic.plist" />
            <BlendFunc Src="1" Dst="771" />
          </AbstractNodeData>
          <AbstractNodeData Name="Bush_live" ActionTag="462940095" VisibleForFrame="False" Tag="40" IconVisible="False" LeftMargin="564.5389" RightMargin="566.4611" TopMargin="320.3669" BottomMargin="309.6331" ctype="SpriteObjectData">
            <Size X="149.0000" Y="90.0000" />
            <AnchorPoint ScaleX="0.5000" ScaleY="0.5000" />
            <Position X="639.0389" Y="354.6331" />
            <Scale ScaleX="1.0000" ScaleY="1.0000" />
            <CColor A="255" R="255" G="255" B="255" />
            <PrePosition X="0.4992" Y="0.4925" />
            <PreSize X="0.1164" Y="0.1250" />
            <FileData Type="PlistSubImage" Path="Bush2.png" Plist="box2d_pic.plist" />
            <BlendFunc Src="1" Dst="771" />
          </AbstractNodeData>
          <AbstractNodeData Name="water" ActionTag="-465788934" Tag="41" IconVisible="False" LeftMargin="590.0000" RightMargin="590.0000" TopMargin="139.9666" BottomMargin="480.0334" ctype="SpriteObjectData">
            <Size X="100.0000" Y="100.0000" />
            <AnchorPoint ScaleX="0.5000" ScaleY="0.5000" />
            <Position X="640.0000" Y="530.0334" />
            <Scale ScaleX="0.5000" ScaleY="0.5000" />
            <CColor A="255" R="255" G="255" B="255" />
            <PrePosition X="0.5000" Y="0.7362" />
            <PreSize X="0.0781" Y="0.1389" />
            <FileData Type="PlistSubImage" Path="water.png" Plist="box2d_pic.plist" />
            <BlendFunc Src="1" Dst="771" />
          </AbstractNodeData>
          <AbstractNodeData Name="btn_start" ActionTag="1832383669" Tag="27" IconVisible="False" LeftMargin="455.9850" RightMargin="460.0150" TopMargin="493.0952" BottomMargin="46.9048" TouchEnable="True" FontSize="14" Scale9Enable="True" LeftEage="15" RightEage="15" TopEage="11" BottomEage="11" Scale9OriginX="15" Scale9OriginY="11" Scale9Width="334" Scale9Height="158" ShadowOffsetX="2.0000" ShadowOffsetY="-2.0000" ctype="ButtonObjectData">
            <Size X="364.0000" Y="180.0000" />
            <AnchorPoint ScaleX="0.5000" ScaleY="0.5000" />
            <Position X="637.9850" Y="136.9048" />
            <Scale ScaleX="0.5000" ScaleY="0.5000" />
            <CColor A="255" R="255" G="255" B="255" />
            <PrePosition X="0.4984" Y="0.1901" />
            <PreSize X="0.2844" Y="0.2500" />
            <TextColor A="255" R="65" G="65" B="70" />
            <DisabledFileData Type="Default" Path="Default/Button_Disable.png" Plist="" />
            <PressedFileData Type="PlistSubImage" Path="btn_start_2.png" Plist="box2d_bg_pic.plist" />
            <NormalFileData Type="PlistSubImage" Path="btn_start_0.png" Plist="box2d_bg_pic.plist" />
            <OutlineColor A="255" R="255" G="0" B="0" />
            <ShadowColor A="255" R="110" G="110" B="110" />
          </AbstractNodeData>
          <AbstractNodeData Name="btn_setting" ActionTag="-1525753659" Tag="28" IconVisible="False" LeftMargin="145.8139" RightMargin="770.1861" TopMargin="493.0952" BottomMargin="46.9048" TouchEnable="True" FontSize="14" Scale9Enable="True" LeftEage="15" RightEage="15" TopEage="11" BottomEage="11" Scale9OriginX="15" Scale9OriginY="11" Scale9Width="334" Scale9Height="158" ShadowOffsetX="2.0000" ShadowOffsetY="-2.0000" ctype="ButtonObjectData">
            <Size X="364.0000" Y="180.0000" />
            <AnchorPoint ScaleX="0.5000" ScaleY="0.5000" />
            <Position X="327.8139" Y="136.9048" />
            <Scale ScaleX="0.5000" ScaleY="0.5000" />
            <CColor A="255" R="255" G="255" B="255" />
            <PrePosition X="0.2561" Y="0.1901" />
            <PreSize X="0.2844" Y="0.2500" />
            <TextColor A="255" R="65" G="65" B="70" />
            <DisabledFileData Type="Default" Path="Default/Button_Disable.png" Plist="" />
            <PressedFileData Type="PlistSubImage" Path="btn_setting_2.png" Plist="box2d_bg_pic.plist" />
            <NormalFileData Type="PlistSubImage" Path="btn_setting_0.png" Plist="box2d_bg_pic.plist" />
            <OutlineColor A="255" R="255" G="0" B="0" />
            <ShadowColor A="255" R="110" G="110" B="110" />
          </AbstractNodeData>
          <AbstractNodeData Name="btn_exit" ActionTag="-999866087" Tag="29" IconVisible="False" LeftMargin="766.1549" RightMargin="149.8451" TopMargin="493.0952" BottomMargin="46.9048" TouchEnable="True" FontSize="14" Scale9Enable="True" LeftEage="15" RightEage="15" TopEage="11" BottomEage="11" Scale9OriginX="15" Scale9OriginY="11" Scale9Width="334" Scale9Height="158" ShadowOffsetX="2.0000" ShadowOffsetY="-2.0000" ctype="ButtonObjectData">
            <Size X="364.0000" Y="180.0000" />
            <AnchorPoint ScaleX="0.5000" ScaleY="0.5000" />
            <Position X="948.1549" Y="136.9048" />
            <Scale ScaleX="0.5000" ScaleY="0.5000" />
            <CColor A="255" R="255" G="255" B="255" />
            <PrePosition X="0.7407" Y="0.1901" />
            <PreSize X="0.2844" Y="0.2500" />
            <TextColor A="255" R="65" G="65" B="70" />
            <DisabledFileData Type="Default" Path="Default/Button_Disable.png" Plist="" />
            <PressedFileData Type="PlistSubImage" Path="btn_exit_2.png" Plist="box2d_bg_pic.plist" />
            <NormalFileData Type="PlistSubImage" Path="btn_exit_0.png" Plist="box2d_bg_pic.plist" />
            <OutlineColor A="255" R="255" G="0" B="0" />
            <ShadowColor A="255" R="110" G="110" B="110" />
          </AbstractNodeData>
          <AbstractNodeData Name="black_bg" ActionTag="-1422161813" VisibleForFrame="False" Alpha="229" Tag="43" IconVisible="False" LeftMargin="617.0000" RightMargin="617.0000" TopMargin="337.0000" BottomMargin="337.0000" ctype="SpriteObjectData">
            <Size X="1284.0000" Y="964.0000" />
            <AnchorPoint ScaleX="0.5000" ScaleY="0.5000" />
            <Position X="640.0000" Y="360.0000" />
            <Scale ScaleX="1.0000" ScaleY="0.7533" />
            <CColor A="255" R="96" G="96" B="96" />
            <PrePosition X="0.5000" Y="0.5000" />
            <PreSize X="0.0359" Y="0.0639" />
            <FileData Type="Normal" Path="background.png" Plist="" />
            <BlendFunc Src="1" Dst="771" />
          </AbstractNodeData>
          <AbstractNodeData Name="setting_window" ActionTag="-651860507" VisibleForFrame="False" Tag="30" IconVisible="False" LeftMargin="329.9992" RightMargin="330.0008" TopMargin="153.4998" BottomMargin="141.5002" ctype="SpriteObjectData">
            <Size X="620.0000" Y="425.0000" />
            <AnchorPoint ScaleX="0.5000" ScaleY="0.5000" />
            <Position X="639.9992" Y="354.0002" />
            <Scale ScaleX="0.9257" ScaleY="0.6931" />
            <CColor A="255" R="255" G="255" B="255" />
            <PrePosition X="0.5000" Y="0.4917" />
            <PreSize X="0.4844" Y="0.5903" />
            <FileData Type="PlistSubImage" Path="windows1.png" Plist="box2d_bg_pic.plist" />
            <BlendFunc Src="1" Dst="771" />
          </AbstractNodeData>
          <AbstractNodeData Name="btn_easy" ActionTag="-1348472966" VisibleForFrame="False" Tag="31" IconVisible="False" LeftMargin="395.1732" RightMargin="703.8268" TopMargin="267.9207" BottomMargin="270.0793" TouchEnable="True" FontSize="14" Scale9Enable="True" LeftEage="15" RightEage="15" TopEage="11" BottomEage="11" Scale9OriginX="15" Scale9OriginY="11" Scale9Width="151" Scale9Height="160" ShadowOffsetX="2.0000" ShadowOffsetY="-2.0000" ctype="ButtonObjectData">
            <Size X="181.0000" Y="182.0000" />
            <AnchorPoint ScaleX="0.5000" ScaleY="0.5000" />
            <Position X="485.6732" Y="361.0793" />
            <Scale ScaleX="0.4000" ScaleY="0.4000" />
            <CColor A="255" R="255" G="255" B="255" />
            <PrePosition X="0.3794" Y="0.5015" />
            <PreSize X="0.1414" Y="0.2528" />
            <TextColor A="255" R="65" G="65" B="70" />
            <DisabledFileData Type="PlistSubImage" Path="btn_level1_2.png" Plist="box2d_bg_pic.plist" />
            <PressedFileData Type="PlistSubImage" Path="btn_level1_2.png" Plist="box2d_bg_pic.plist" />
            <NormalFileData Type="PlistSubImage" Path="btn_level1_0.png" Plist="box2d_bg_pic.plist" />
            <OutlineColor A="255" R="255" G="0" B="0" />
            <ShadowColor A="255" R="110" G="110" B="110" />
          </AbstractNodeData>
          <AbstractNodeData Name="btn_normal" ActionTag="79494613" VisibleForFrame="False" Tag="32" IconVisible="False" LeftMargin="499.1892" RightMargin="599.8108" TopMargin="267.9207" BottomMargin="270.0793" TouchEnable="True" FontSize="14" Scale9Enable="True" LeftEage="15" RightEage="15" TopEage="11" BottomEage="11" Scale9OriginX="15" Scale9OriginY="11" Scale9Width="151" Scale9Height="160" ShadowOffsetX="2.0000" ShadowOffsetY="-2.0000" ctype="ButtonObjectData">
            <Size X="181.0000" Y="182.0000" />
            <AnchorPoint ScaleX="0.5000" ScaleY="0.5000" />
            <Position X="589.6892" Y="361.0793" />
            <Scale ScaleX="0.4000" ScaleY="0.4000" />
            <CColor A="255" R="255" G="255" B="255" />
            <PrePosition X="0.4607" Y="0.5015" />
            <PreSize X="0.1414" Y="0.2528" />
            <TextColor A="255" R="65" G="65" B="70" />
            <DisabledFileData Type="PlistSubImage" Path="btn_level2_2.png" Plist="box2d_bg_pic.plist" />
            <PressedFileData Type="PlistSubImage" Path="btn_level2_2.png" Plist="box2d_bg_pic.plist" />
            <NormalFileData Type="PlistSubImage" Path="btn_level2_0.png" Plist="box2d_bg_pic.plist" />
            <OutlineColor A="255" R="255" G="0" B="0" />
            <ShadowColor A="255" R="110" G="110" B="110" />
          </AbstractNodeData>
          <AbstractNodeData Name="btn_hard" ActionTag="-285819423" VisibleForFrame="False" Tag="33" IconVisible="False" LeftMargin="603.2059" RightMargin="495.7941" TopMargin="267.9207" BottomMargin="270.0793" TouchEnable="True" FontSize="14" Scale9Enable="True" LeftEage="15" RightEage="15" TopEage="11" BottomEage="11" Scale9OriginX="15" Scale9OriginY="11" Scale9Width="151" Scale9Height="160" ShadowOffsetX="2.0000" ShadowOffsetY="-2.0000" ctype="ButtonObjectData">
            <Size X="181.0000" Y="182.0000" />
            <AnchorPoint ScaleX="0.5000" ScaleY="0.5000" />
            <Position X="693.7059" Y="361.0793" />
            <Scale ScaleX="0.4000" ScaleY="0.4000" />
            <CColor A="255" R="255" G="255" B="255" />
            <PrePosition X="0.5420" Y="0.5015" />
            <PreSize X="0.1414" Y="0.2528" />
            <TextColor A="255" R="65" G="65" B="70" />
            <DisabledFileData Type="PlistSubImage" Path="btn_level3_2.png" Plist="box2d_bg_pic.plist" />
            <PressedFileData Type="PlistSubImage" Path="btn_level3_2.png" Plist="box2d_bg_pic.plist" />
            <NormalFileData Type="PlistSubImage" Path="btn_level3_0.png" Plist="box2d_bg_pic.plist" />
            <OutlineColor A="255" R="255" G="0" B="0" />
            <ShadowColor A="255" R="110" G="110" B="110" />
          </AbstractNodeData>
          <AbstractNodeData Name="btn_final" ActionTag="1587307126" VisibleForFrame="False" Tag="34" IconVisible="False" LeftMargin="707.2216" RightMargin="391.7784" TopMargin="267.9207" BottomMargin="270.0793" TouchEnable="True" FontSize="14" Scale9Enable="True" LeftEage="15" RightEage="15" TopEage="11" BottomEage="11" Scale9OriginX="15" Scale9OriginY="11" Scale9Width="151" Scale9Height="160" ShadowOffsetX="2.0000" ShadowOffsetY="-2.0000" ctype="ButtonObjectData">
            <Size X="181.0000" Y="182.0000" />
            <AnchorPoint ScaleX="0.5000" ScaleY="0.5000" />
            <Position X="797.7216" Y="361.0793" />
            <Scale ScaleX="0.4000" ScaleY="0.4000" />
            <CColor A="255" R="255" G="255" B="255" />
            <PrePosition X="0.6232" Y="0.5015" />
            <PreSize X="0.1414" Y="0.2528" />
            <TextColor A="255" R="65" G="65" B="70" />
            <DisabledFileData Type="PlistSubImage" Path="btn_level4_2.png" Plist="box2d_bg_pic.plist" />
            <PressedFileData Type="PlistSubImage" Path="btn_level4_2.png" Plist="box2d_bg_pic.plist" />
            <NormalFileData Type="PlistSubImage" Path="btn_level4_0.png" Plist="box2d_bg_pic.plist" />
            <OutlineColor A="255" R="255" G="0" B="0" />
            <ShadowColor A="255" R="110" G="110" B="110" />
          </AbstractNodeData>
          <AbstractNodeData Name="btn_okay" ActionTag="-1642017225" VisibleForFrame="False" Tag="35" IconVisible="False" LeftMargin="458.0000" RightMargin="458.0000" TopMargin="420.9995" BottomMargin="119.0005" TouchEnable="True" FontSize="14" Scale9Enable="True" LeftEage="15" RightEage="15" TopEage="11" BottomEage="11" Scale9OriginX="15" Scale9OriginY="11" Scale9Width="334" Scale9Height="158" ShadowOffsetX="2.0000" ShadowOffsetY="-2.0000" ctype="ButtonObjectData">
            <Size X="364.0000" Y="180.0000" />
            <AnchorPoint ScaleX="0.5000" ScaleY="0.5000" />
            <Position X="640.0000" Y="209.0005" />
            <Scale ScaleX="0.5000" ScaleY="0.5000" />
            <CColor A="255" R="255" G="255" B="255" />
            <PrePosition X="0.5000" Y="0.2903" />
            <PreSize X="0.2844" Y="0.2500" />
            <TextColor A="255" R="65" G="65" B="70" />
            <DisabledFileData Type="Default" Path="Default/Button_Disable.png" Plist="" />
            <PressedFileData Type="PlistSubImage" Path="btn_okay_2.png" Plist="box2d_bg_pic.plist" />
            <NormalFileData Type="PlistSubImage" Path="btn_okay_0.png" Plist="box2d_bg_pic.plist" />
            <OutlineColor A="255" R="255" G="0" B="0" />
            <ShadowColor A="255" R="110" G="110" B="110" />
          </AbstractNodeData>
        </Children>
      </ObjectData>
    </Content>
  </Content>
</GameFile>