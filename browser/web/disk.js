var refreash_index_dialog_timeout;

function drive_type_name(type){
	if (type==0) return "未知"
	if (type==1) return "无效"
	if (type==2) return "U盘"
	if (type==3) return "硬盘"
	if (type==4) return "网络磁盘"
	if (type==5) return "光盘"
	if (type==6) return "内存盘"
	return ""
}

function contain(id, obj){
		var ret=false;
		$.each(obj, function (index, ele) { 
			if(ele.id == id){
				ret = true;
			}
		});
		return ret;
}

function get_drive_info(id, d_infos){
	for(var f_index=0; f_index<d_infos.length; f_index++){
		if(d_infos[f_index].id == id) return d_infos[f_index];
	}
}

function gen_offline_dir_name(file, d_infos){
	var id = file.path.substring(0,2);
	if(id>=26 && id<26*3){
		var dir = file.path.substring(2, file.path.length);
		var info = get_drive_info(id, d_infos);
		if(info.volumeName) disk_name = info.volumeName;
		else disk_name = '未命名'+(id-25);
		file.path = "["+drive_type_name(info.type)+":"+disk_name+"]"+dir;
	}
}

function get_loaded_offline_dbs(){
	var done = eval(cef.plugin.search("[///index_status"));
	var drives = eval(cef.plugin.search("[///get_drives"));
	var offline = [];
	$.each(done, function (index, ele) { 
		if(!contain(ele.id,drives)){
			offline.push(ele);
		}
	});
	return offline;
}


function get_offline_dbs(){
	var dbs = eval(cef.plugin.search("[///cache_dbs"));
	dbs_name = dbs.map(function(ele){
		return ele.name;
	})
	var drives = eval(cef.plugin.search("[///get_drives"));
	$.each(drives, function (index, ele) { 
		var i = $.inArray(ele.serialNumber+".db", dbs_name);
		if(i!=-1) dbs_name.splice(i, 1);
	});
	return dbs_name;
}

function add_time_info(objs){
	var dbs = eval(cef.plugin.search("[///cache_dbs"));
	dbs_name = dbs.map(function(ele){
		return ele.name;
	})
	$.each(objs, function (index, ele) { 
		var i = $.inArray(ele.serialNumber+".db", dbs_name);
		if(i!=-1){
			d = new Date(dbs[i].time*1000);
			ele.time = d.getFullYear()+
		"-"+(d.getMonth()+1)+"-"+d.getDate()+ " "+d.getHours()+":"+d.getMinutes();
		}
	});
}

function parse_space(objs){
	$.each(objs, function (index, ele) {
		if(ele.totalMB>9000){
			ele.totalMB = Math.round(ele.totalMB/1000) + "GB";
			ele.totalFreeMB = Math.round(ele.totalFreeMB/1000) + "GB";
		} else if(ele.totalMB>900){
			ele.totalMB = Math.round(ele.totalMB/100)/10 + "GB";
			ele.totalFreeMB = Math.round(ele.totalFreeMB/100)/10 + "GB";
		}else{
			ele.totalMB = Math.round(ele.totalMB/10)*10 + "MB";
			ele.totalFreeMB = Math.round(ele.totalFreeMB/10)*10 + "MB";
		}
	});
}

//判断是否有离线db
function has_offline_dbs(){
	return get_offline_dbs().length>0;
}

function todo_dirves(){
	var done = eval(cef.plugin.search("[///index_status"));
	var drives = eval(cef.plugin.search("[///get_drives"));
	done_sn = done.map(function(ele){
		return ele.serialNumber;
	})
	drive_sn = drives.map(function(ele){
		return ele.serialNumber;
	})
	$.each(done_sn, function (index, ele) { 
		var i = $.inArray(ele, drive_sn);
		if(i!=-1) drive_sn.splice(i, 1);
	});
	var ret = [];
	$.each(drive_sn, function (index, ele) { 
		if(ele.length>0 && ele!="0") ret.push(ele);
	});
	return ret;
}

function is_scan_finished(){
	return todo_dirves().length == 0
}

function scan_img(){
	if(is_scan_finished()){
		$("#scanning").css("visibility","hidden");
		return;
	}
	$("#scanning").css("visibility","visible");
	setTimeout(scan_img, 5000);
}

function show_index_status(){
	var done = eval(cef.plugin.search("[///index_status"));
	var drives = eval(cef.plugin.search("[///get_drives"));
	$.each(drives, function (index, ele) { 
		if(!ele.volumeName) ele.volumeName="未命名";
		ele.panfu = String.fromCharCode(65+ele.id*1);
		ele.typename = drive_type_name(ele.type);
		if(contain(ele.id,done)){
			ele.indexed = "images/tick.png";
		}else{
			if(ele.serialNumber.length>1) ele.indexed = "images/spinner.gif";
			else  ele.indexed = "";
		}
		if(!ele.fsName) ele.display="display:none";
	});
	var offline = get_loaded_offline_dbs();
	$.each(offline, function (index, ele) { 
		ele.indexed = "离线";
		ele.typename = drive_type_name(ele.type);
		if(!ele.volumeName) ele.volumeName = '未命名 ('+(ele.id-25)+')';
	});
	$("#index_status_result").setTemplateElement("index_status_template");
	add_time_info(drives);
	parse_space(drives);
	$("#index_status_result").processTemplate(drives);
	$("#offline-index_status_result").setTemplateElement("offline_index_status_template");
	add_time_info(offline);
	parse_space(offline);
	$("#offline-index_status_result").processTemplate(offline);
	$("#dialog-index-status").dialog({modal: true, width:600});
}

function refreash_index_status(){
	if($("#dialog-index-status").parent().css("display")=="block"){
		show_index_status();
		refreash_index_dialog_timeout = setTimeout(refreash_index_status,1000);
	}
}

function show_hotkey(){
	$("#select-hotkey").val(cef.plugin.hotkey);
	$("#dialog-set-hotkey").dialog({modal: true, width:600});
}
function set_hotkey(){
	cef.plugin.hotkey = $("#select-hotkey").val();
	$("#dialog-set-hotkey").dialog("close");
	show_info("快捷键设置成功！");
}

function show_export(){
	d = new Date();
	$("#export-filename").val("c:\\gigaso-"+d.getFullYear()+
		"-"+(d.getMonth()+1)+"-"+d.getDate() +"'"+d.getSeconds() +".txt");
	$("#dialog-export").dialog({modal: true, width:600});
}
function do_export(){
	var etype = $("#select-export").val();
	var s = "";
	$.each(files, function (index, ele) { 
		if(ele.path2) s += ele.path2;
		else  s += ele.path;
		if(ele.name2) s += ele.name2;
		else s += ele.name;
		if(etype=="1"){
			s += "\t,"+ele.size+"\t,"+ele.time;
		}
		s += "\r\n";
	});
	cef.plugin.save($("#export-filename").val(),s);
	$("#dialog-export").dialog("close");
	show_info("查询结果导出成功！");
}

function rescan(td, i){
	clearTimeout(refreash_index_dialog_timeout);
	$(td.parentNode.cells(td.cellIndex-1)).html('<img src="images/spinner.gif">');
	cef.plugin.search("[///rescan"+i);
	refreash_index_dialog_timeout = setTimeout(refreash_index_status,3000);
}

function del_offline_db(td,i){
	$(td).html('<img src="images/spinner.gif">');
	function rescan0(){
		cef.plugin.search("[///del_offline_db"+i);
		$("#dialog-index-status").dialog("close");
		show_index_status();
	}	
	setTimeout(rescan0,10);
}

function offline_db(){
	$("#loading").css("visibility","visible");
	cef.plugin.offline = true;
	cef.plugin.search("[///load_offline_db");
	//判断离线DB是否加载完成
	setTimeout(refresh,2000);
}
function online_db(){
	cef.plugin.offline = false;
	setTimeout(refresh,10);
}

function upgrade_req(){
	var data = "upgrade[os]="+cef.gigaso.os+"&upgrade[cpu]="+cef.gigaso.cpu+"&upgrade[disk]="
		+cef.gigaso.disk+"&upgrade[ver]="+cef.gigaso.ver+"&upgrade[user]="+cef.gigaso.user;
	var jqxhr = $.ajax({
	  type: 'POST',
	  url: host+"/upgrades.js",
	  data: data,
	  dataType: "json"
	})
    .success(function(data, textStatus, jqXHR) { 
		if(data.status==1)
			cef.plugin.search("[///upgrade"+host+data.url+"?"+data.hash+"&"+data.version);
		else cef.plugin.search("[///upgrade_none");
	 })
    .error(function(jqXHR, textStatus, errorThrown) {
		console.log(textStatus);
		console.log(jqXHR.responseText);
		console.log(errorThrown);
	});
}

function show_upgrade(){
	var jqxhr = $.ajax({
	  type: 'GET',
	  url: host+"/gigasos.js?version="+cef.gigaso.ver_new,
	  dataType: "json"
	})
    .success(function(data, textStatus, jqXHR) { 
	    $('#upgrade_desc').html(data.desc);
		$("#old_version").html(cef.gigaso.ver);
		$("#new_version").html(cef.gigaso.ver_new);
		$("#dialog-upgrade").dialog({modal: true, width:600});
	 });
}

function do_upgrade(){
	$("#dialog-upgrade").dialog("close");
	cef.gigaso.do_update();
}

function check_upgrade(){
		var update_status = cef.gigaso.check_update();
		if(update_status==0){
			upgrade_req();
		}else if(update_status==1){
			show_upgrade();
		}
}

var first_grid=true;
function file_grid(){
	$("#maintable").jqGrid({ data: files, 
						datatype: "local", 
						width : "1000",
						height: "auto",
						pager: false,
						colNames:['文件名','文件夹','大小','修改日期',''], 
						colModel:[ 
									{name:'name', width:"40%",sortable:false },
									{name:'path', width:"38%",sortable:false},
									{name:'size', width:"7%", align:"right",sortable:false},
									{name:'time', width:"15%", align:"right",sortable:false},
									{name:'type', width:"0%", hidden:true,sortable:false}
								], 
						rowNum:201,
						gridview: true,
						forceFit : true,
						scrollrows: false,
						multiselect: false,
						/*
							gridComplete : function() {
								var tm = jQuery("#maintable").jqGrid('getGridParam','totaltime');
								$("#dir").val("Render time: "+ tm+" ms ");
							},
						*/
					   onSelectRow: function(id,status){ 
						  idd=id*1+1; //id从1起，但是jggrow从2起
						  if(status){
							if($("#maintable").jqGrid('getGridParam','selarrrow').length>1)
						      $(".jqgrow:nth-child("+idd+")").contextMenu('myMenu3', context_menu_obj3);
						  }else{
						   $(".jqgrow:nth-child("+idd+")").contextMenu('myMenu1', context_menu_obj);
						  }
					   },
						sortable: false});
	$(".ui-jqgrid-htable th").bind('click',function(event,t){
		if(this.id=="maintable_name") var index=0;
		if(this.id=="maintable_path") var index=1;
		if(this.id=="maintable_size") var index=2;
		if(this.id=="maintable_time") var index=3;
		if(index==order_col){
			orderby(index,!order_desc);
		}else{
			orderby(index,1);
		}
	});
	first_grid=false;
	set_order_arrow(order_col+1,order_desc);
	grid_auto_width();
	gird_event();
}
function gird_event(){
	$("#maintable").setGridParam({multiselect: true});
	$(window).bind('resize',function(event){
		grid_auto_width();
	});
	$("#maintable tr:nth-child(even)").addClass("d");
	$("#maintable").jqGrid('bindKeys', {
		"onEnter":function( rowid ) {dblclick_file($(".jqgrow", "#maintable")[rowid]);} ,
		scrollingRows: false
	} );
	if(!cef.plugin.offline){
		$(".jqgrow", "#maintable").contextMenu('myMenu1', context_menu_obj);
		$(".jqgrow td:nth-child(1)").bind('dblclick',function(e){
			dblclick_file(e.currentTarget.parentNode);
		});
		$(".jqgrow td:nth-child(2)").bind('dblclick',function(e){
			dblclick_path(e.currentTarget.parentNode);
		});
	}else{
		$(".jqgrow", "#maintable").contextMenu('myMenu2', context_menu_obj);
	}
	$(".jqgrow", "#maintable").each(function(index,e){
		var s = files[index].icon+$(e).find("td")[0].innerHTML;
		$(e).find("td")[0].innerHTML = s;
	});
	highlight_timeout = setTimeout(highlight,1);
}
function grid_auto_width(){
	$("#maintable").jqGrid('setGridWidth',$("#tab-1").width());
}
function view_grid(){
	if(first_grid) file_grid();
	else{
		$("#maintable").setGridParam({multiselect: false});
		$("#maintable").clearGridData();
		$("#maintable").setGridParam({data: files}).trigger("reloadGrid");
		gird_event();
	}
}

function gird_selectd_files(){
	var selectd_file_ids = $("#maintable").jqGrid('getGridParam','selarrrow');
	var ret="";
	$.each(selectd_file_ids, function (index, ele) { 
		var file = files[ele-1];
		ret += file.path;
		ret += file.name;
		ret += "|";
	});
	return ret;
}