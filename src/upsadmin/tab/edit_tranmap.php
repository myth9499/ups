<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
<title>交易码修改</title>
<style type="text/css">
<!--
body {
	margin-left: 0px;
	margin-top: 0px;
	margin-right: 0px;
	margin-bottom: 0px;
}
.STYLE1 {font-size: 12px}
.STYLE3 {font-size: 12px; font-weight: bold; }
.STYLE4 {
	color: #03515d;
	font-size: 12px;
}
-->
</style>

<script>
var  highlightcolor='#c1ebff';
//�˴�clickcolorֻ����winϵͳ��ɫ������ܳɹ�,�����#xxxxxx�Ĵ���Ͳ���,��û�����Ϊʲô:(
var  clickcolor='#51b2f6';
function  changeto(){
source=event.srcElement;
if  (source.tagName=="TR"||source.tagName=="TABLE")
return;
while(source.tagName!="TD")
source=source.parentElement;
source=source.parentElement;
cs  =  source.children;
//alert(cs.length);
if  (cs[1].style.backgroundColor!=highlightcolor&&source.id!="nc"&&cs[1].style.backgroundColor!=clickcolor)
for(i=0;i<cs.length;i++){
	cs[i].style.backgroundColor=highlightcolor;
}
}

function  changeback(){
if  (event.fromElement.contains(event.toElement)||source.contains(event.toElement)||source.id=="nc")
return
if  (event.toElement!=source&&cs[1].style.backgroundColor!=clickcolor)
//source.style.backgroundColor=originalcolor
for(i=0;i<cs.length;i++){
	cs[i].style.backgroundColor="";
}
}

function  clickto(){
source=event.srcElement;
if  (source.tagName=="TR"||source.tagName=="TABLE")
return;
while(source.tagName!="TD")
source=source.parentElement;
source=source.parentElement;
cs  =  source.children;
//alert(cs.length);
if  (cs[1].style.backgroundColor!=clickcolor&&source.id!="nc")
for(i=0;i<cs.length;i++){
	cs[i].style.backgroundColor=clickcolor;
}
else
for(i=0;i<cs.length;i++){
	cs[i].style.backgroundColor="";
}
}
function updatetranmap(id)
{
	alert(id);
	alert(document.getElementById("trancode").value);
	alert(document.getElementById("tranname").value);
	alert(document.getElementById("tranflow").value);
	alert(document.getElementById("timeout").value);
	
}
</script>

</head>

<body>
<table width="100%" border="0" cellspacing="0" cellpadding="0">
  <tr>
    <td height="30" background="images/tab_05.gif"><table width="100%" border="0" cellspacing="0" cellpadding="0">
      <tr>
        <td width="12" height="30"><img src="images/tab_03.gif" width="12" height="30" /></td>
        <td><table width="100%" border="0" cellspacing="0" cellpadding="0">
          <tr>
            <td width="46%" valign="middle"><table width="100%" border="0" cellspacing="0" cellpadding="0">
              <tr>
                <td width="5%"><div align="center"><img src="images/tb.gif" width="16" height="16" /></div></td>
                <td width="95%" class="STYLE1"><span class="STYLE3">当前位置</span>：[业务中心]-[交易码配置]</td>
              </tr>
            </table></td>
            <td width="54%"><table border="0" align="right" cellpadding="0" cellspacing="0">
              <tr>
                <td width="60"><table width="87%" border="0" cellpadding="0" cellspacing="0">
                  <tr>
                    <td class="STYLE1"><div align="center">
                      <input type="checkbox" name="checkbox62" value="checkbox" />
                    </div></td>
                    <td class="STYLE1"><div align="center">全选</div></td>
                  </tr>
                </table></td>
                <td width="60"><table width="90%" border="0" cellpadding="0" cellspacing="0">
                  <tr>
                    <td class="STYLE1"><div align="center"><img src="images/22.gif" width="14" height="14" /></div></td>
                    <td class="STYLE1"><div align="center">新增</div></td>
                  </tr>
                </table></td>
                <td width="60"><table width="90%" border="0" cellpadding="0" cellspacing="0">
                  <tr>
                    <td class="STYLE1"><div align="center"><img src="images/33.gif" width="14" height="14" /></div></td>
                    <td class="STYLE1"><div align="center">修改</div></td>
                  </tr>
                </table></td>
                <td width="52"><table width="88%" border="0" cellpadding="0" cellspacing="0">
                  <tr>
                    <td class="STYLE1"><div align="center"><img src="images/11.gif" width="14" height="14" /></div></td>
                    <td class="STYLE1"><div align="center">删除</div></td>
                  </tr>
                </table></td>
              </tr>
            </table></td>
          </tr>
        </table></td>
        <td width="16"><img src="images/tab_07.gif" width="16" height="30" /></td>
      </tr>
    </table></td>
  </tr>
  <tr>
    <td><table width="100%" border="0" cellspacing="0" cellpadding="0">
      <tr>
        <td width="8" background="images/tab_12.gif">&nbsp;</td>
        <td><table width="100%" border="0" cellpadding="0" cellspacing="1" bgcolor="b5d6e6" onmouseover="changeto()"  onmouseout="changeback()">
          <tr>
            <td width="3%" height="22" background="images/bg.gif" bgcolor="#FFFFFF"><div align="center"><span class="STYLE1">序号</span></div></td>
            <td width="10%" height="22" background="images/bg.gif" bgcolor="#FFFFFF"><div align="center"><span class="STYLE1">交易码</span></div></td>
            <td width="5%" height="22" background="images/bg.gif" bgcolor="#FFFFFF"><div align="center"><span class="STYLE1">交易名称</span></div></td>
            <td width="15%" height="22"background="images/bg.gif" bgcolor="#FFFFFF"><div align="center"><span class="STYLE1">交易流程</span></div></td>
            <td width="3%" height="22" background="images/bg.gif" bgcolor="#FFFFFF"><div align="center"><span class="STYLE1">超时时间</span></div></td>
			<td width="3%" height="22" background="images/bg.gif" bgcolor="#FFFFFF" class="STYLE1"><div align="center">基本操作</div></td>
          </tr>
			<!---html
            <td height="20" bgcolor="#FFFFFF"><div align="center" class="STYLE1">
              <div align="center">01</div>
            </div></td>
			-->
<?php
	$id = $_GET['id'];
	$cnt = 0;
    $db = new SQLite3("/item/ups/src/cfg/db/ups.sqlite");
	$statement = $db->prepare('SELECT * FROM tranmap  where id=:id;');
	$statement->bindValue(':id', $id);

	$results = $statement->execute();
	while ($row = $results->fetchArray()) {
		$cnt++;
     echo "<tr>";
     echo "<td height='20' bgcolor='#FFFFFF'><div align='center'><span class='STYLE1'>".$row[0]."</span></div></td>";
	 echo "<td height='20' bgcolor='#FFFFFF'><div align='center'><span class='STYLE1'>"."<input type = 'text' id = 'trancode' value='$row[1]' />"."</span></div></td>";
	 echo "<td height='20' bgcolor='#FFFFFF'><div align='center'><span class='STYLE1'>"."<input type = 'text' id = 'tranname' value='$row[2]' />"."</span></div></td>";
	 echo "<td height='20' bgcolor='#FFFFFF'><div align='center'><span class='STYLE1'>"."<input type = 'text' id = 'tranflow' value='$row[3]' />"."</span></div></td>";
	 echo "<td height='20' bgcolor='#FFFFFF'><div align='center'><span class='STYLE1'>"."<input type = 'text' id = 'timeout' value='$row[4]' />"."</span></div></td>";
   echo "<td height='20' bgcolor='#FFFFFF'><div align='center'><span class='STYLE4'><input type='image'  src='../images/sure.gif' onclick='updatetranmap($row[0])' width='22' height='16' /></span></div></td>";
   //echo "<td height='20' bgcolor='#FFFFFF'><div align='center'><span class='STYLE4'><button onclick='updatetranmap($row[0])' width='16' height='16' />提交</button></span></div></td>";
   echo "</tr>";
	}
?>
        </table></td>
        <td width="8" background="images/tab_15.gif">&nbsp;</td>
      </tr>
    </table></td>
  </tr>
  <tr>
    <td height="35" background="images/tab_19.gif"><table width="100%" border="0" cellspacing="0" cellpadding="0">
      <tr>
        <td width="12" height="35"><img src="images/tab_18.gif" width="12" height="35" /></td>
        <td><table width="100%" border="0" cellspacing="0" cellpadding="0">
          <tr>
		  <?php
            echo "<td class='STYLE4'>&nbsp;&nbsp;共有 ".$cnt."条记录，当前第 1/10 页</td>";
			?>
            <td><table border="0" align="right" cellpadding="0" cellspacing="0">
                <tr>
                  <td width="40"><img src="images/first.gif" width="37" height="15" /></td>
                  <td width="45"><img src="images/back.gif" width="43" height="15" /></td>
                  <td width="45"><img src="images/next.gif" width="43" height="15" /></td>
                  <td width="40"><img src="images/last.gif" width="37" height="15" /></td>
                  <td width="100"><div align="center"><span class="STYLE1">转到第
                    <input name="textfield" type="text" size="4" style="height:12px; width:20px; border:1px solid #999999;" /> 
                    页 </span></div></td>
                  <td width="40"><img src="images/go.gif" width="37" height="15" /></td>
                </tr>
            </table></td>
          </tr>
        </table></td>
        <td width="16"><img src="images/tab_20.gif" width="16" height="35" /></td>
      </tr>
    </table></td>
  </tr>
</table>
</body>
</html>
