var file;
var path;
var type;

var num_per_row=4;
var row_per_page=2;
var num_per_page=num_per_row*row_per_page;
var num_all_page=num_per_page*2;

	var host = "http://60.191.119.190:3333";
	var order_col;
	var order_desc;
	var cur_search;
	var files = [];
	var search_timeout;
	var highlight_timeout;
	var history_timeout;
	var stat_timeout;

	function trim(str, chars) {
		return ltrim(rtrim(str, chars), chars);
	}
	 
	function ltrim(str, chars) {
		chars = chars || "\\s";
		return str.replace(new RegExp("^[" + chars + "]+", "g"), "");
	}
	 
	function rtrim(str, chars) {
		chars = chars || "\\s";
		return str.replace(new RegExp("[" + chars + "]+$", "g"), "");
	}
	  var icon_replace_index=0;
	  function icon_replace(){
			var finished = true;
			$("#maintable tr:gt("+grid_start_id()+")").slice(icon_replace_index,icon_replace_index+20).each(function(i,o){
				get_file_path(o);
				var full_path = path+file;
				if(full_path.indexOf('%')!=-1) return;
				$(o).find("img")[0].src = "gigaso://icon/"+full_path;
				finished = false;
			});
			if(finished) return;
			icon_replace_index+=20;
			highlight_timeout = setTimeout(icon_replace,1);
	  }
	  function highlight(){
			$("#maintable .jqgrow:not(.ped) td:nth-child(1) span").each(function(i,o){
				var html = o.innerHTML;
				var fi = html.toLowerCase().indexOf($("#search").val().toLowerCase());
				if(fi==-1) return;
				o.innerHTML = html.substring(0, fi) + 
					"<span class='highlight-match'>"+html.substring(fi, fi+$("#search").val().length)+"</span>" + 
					html.substring(fi+$("#search").val().length, html.length);
			});
			$("#maintable .jqgrow:not(.ped)").each(function(i,o){
				get_file_path(o);
				var full_path = path+file;
				if(full_path.indexOf('%')!=-1) return;
				if(full_path.match(/exe$/i)!=null || full_path.match(/lnk$/i)!=null) 
					$(o).find("img")[0].src = "gigaso://icon/"+full_path;
			});
			//icon_replace_index=0;
			$("#maintable .jqgrow:not(.ped)").addClass("ped");
			//if(!cef.plugin.offline) highlight_timeout = setTimeout(icon_replace,1);
			$("#loading").css("visibility","hidden");
	  }


	function file_name_trim2(files){
		$(files).each( function sub(i,obj){
			if(obj.name.length>53){
				obj.name2 = obj.name.substring(0, 25)+"..."+obj.name.substring(obj.name.length-25, obj.name.length);
			}else{
				obj.name2 = obj.name;
			}
		});
	}
	function restart_when_error(){
			show_error("搜索服务失去响应。请稍后重试！");
			files = [];
			view_grid();
			cef.plugin.start_server();
	}
	function search_and_parse(query){
		var s = cef.plugin.search(query);
		try{
			files = eval(s);
		}catch(e){
			restart_when_error();
			$("#loading").css("visibility","hidden");
			return;
		}
		if(cef.plugin.offline){
			var d_infos = get_loaded_offline_dbs();
			for(var f_index=0; f_index<files.length; f_index++)
				gen_offline_dir_name(files[f_index],d_infos);
		}
		$.each(files, function (index, ele) { 
			ele.icon_name = '<img width="16" height="16" style="margin-right:3px" src="icons/'+ele.type+'.ico" /><span>'+ele.name+'</span>';
		});
		if(files.length==1000) files.push({type:"unknown", name:"......", path:"超过1000条的记录不显示",size:"",time:""});
	}
	function searchf0(){
		stat_timeout = setTimeout(stat_search,10);
		$("#loading").css("visibility","visible");
		clearTimeout(highlight_timeout);
		search_and_parse(cur_search);
		if(changed()) return; //查询已过期
		view_grid();
	}
    function searchf(value){
		value = trim(value);
		if(value.length==0) return;
		$("#historys").fadeOut(300);
		$("#tabs").fadeIn(500);
		$("#loading").css("visibility","visible");
		cur_search = value;
		clearTimeout(search_timeout);
		clearTimeout(stat_timeout);
		clearTimeout(highlight_timeout);
		$("#tabs ul span").html("0");
		$("#tab-1 span span").html("0");
		if(value==""){
			$("#loading").css("visibility","hidden");
			return;
		}
		if(value.length<=4){
			search_timeout = setTimeout(searchf0,300);
		}else{
			search_timeout = setTimeout(searchf0,150);
		}        
	}
	function changed(){
		value = $("#search").val();
		value = trim(value);
		return value != cur_search;
	}
	function search_if_change(e){
		$("#dialog-welcome").dialog("close");
		if(e.keyCode==13){
			 $("#maintable").setSelection(1);
			 $(".jqgrow", "#maintable")[0].focus();
			 return;
		}
		if(e.keyCode==8){
			if(cur_search == $("#search").val() ){
				remove_dir();
				searchf(cur_search);
				return;
			}
		}		
		if(e.keyCode==46){
			if($("#dir2").val()!="" && $("#search").val()==""){
				if(cur_search!='') cur_search='';
				else remove_dir();
			}
		}
		if(changed()) searchf(value);
	}
	function orderby(col,desc){
		$("#loading").css("visibility","visible");
		set_orderby_value(col,desc);
		if(!first_grid) set_order_arrow(col*1+1,desc);
		if(files.length>0){
			clearTimeout(search_timeout);
			search_timeout = setTimeout(searchf0,5);
		}
	}
	function set_orderby_value(col,desc){
		order_col = col;
		order_desc = desc;
		cef.plugin.order = col*2+desc+1;
	}
	function set_order_arrow(col_add,desc){
		$(".ui-jqgrid-htable th div span").html("");
		var target_th = $(".ui-jqgrid-htable th:nth-child("+col_add+") div");
		if(desc){
			target_th.html(target_th.html()+"<span><img src='images/s_desc.png' /></span>");
		}else{
			target_th.html(target_th.html()+"<span><img src='images/s_asc.png' /></span>");
		}
	}
	function file_type_search(ft){
		$("#loading").css("visibility","visible");
		cef.plugin.file_type=ft;
		if($("#search").val()=="") return;
		clearTimeout(search_timeout);
		search_timeout = setTimeout(searchf0,5);
	}
	function stat_num_set(stats){
		$("#all_file").html(stats.all);
		$("#dir_file").html(stats.dir);
		$("#compress_file").html(stats.compress);
		$("#program_file").html(stats.program);
		$("#media_file").html(stats.media);
		$("#archive_file").html(stats.archive);
		$("#other_file").html(stats.other);
		$("#exe_file").html(stats.exe);
		$("#link_file").html(stats.link);
		$("#script_file").html(stats.script);
		$("#lib_file").html(stats.lib);
		$("#music_file").html(stats.music);
		$("#photo_file").html(stats.photo);
		$("#video_file").html(stats.video);
		$("#animation_file").html(stats.animation);
		$("#office_file").html(stats.office);
		$("#ebook_file").html(stats.ebook);
		$("#htm_file").html(stats.htm);
		$("#text_file").html(stats.text);
	}
	function stat_search(){
		if(changed()) return; //查询已过期
		if(cur_search=="") return;
		var s = cef.plugin.stat(cur_search);
		try{
			var stats = eval("("+s+")");
			stat_num_set(stats);
		}catch(e){
			restart_when_error();
		}
	}
	function history_parse(start_index){
		if(arguments.length==0) start_index=0;
		var hs = cef.plugin.history().replace(/\\/g,"\\\\");
		hs = eval(hs);
		hs=hs.slice(0,num_all_page);
		for( var i=0;i<num_all_page;i++){
			var fname = hs[i].name;
			if(fname.charAt(fname.length-1)=='\\') fname = fname.substr(0,fname.length-1);
			function calc_top(x){if(x>=num_per_page) x-=num_per_page;if(x>=num_per_row) return 320;return 120}
			function calc_left(x){if(x>=num_per_page) x-=num_per_page;return 100+200*(x%num_per_row)}
			hs[i].id = i;
			hs[i].left = calc_left(i);
			hs[i].top = calc_top(i);
			hs[i].path = fname.substring(0, fname.lastIndexOf("\\")+1);
			hs[i].name = fname.substring(fname.lastIndexOf("\\")+1,fname.length);
			if(hs[i].name==""){
				hs[i].thumb = "";
			}else{
					hs[i].thumb = "gigaso://thumb/"+hs[i].path+hs[i].name;
					hs[i].thumb = hs[i].thumb.replace(/\\/g,"\\\\");
					hs[i].thumb = hs[i].thumb.replace(/ /g,"%20");
			}
		}
		file_name_trim2(hs);
		$("#historys").setTemplateElement("history_template");
		hs0 = hs.slice(start_index,start_index+num_per_page);
		$("#historys").processTemplate(hs0);
		$(".thumbnail-wrapper").bind('click',function(e){
			dblclick_file(e.currentTarget.parentNode);
		});
		$(".thumbnail-wrapper").contextMenu('myMenu1',context_menu_obj);
		for( var i=0;i<hs0.length;i++){
			var p = $(".edit-bar .pin")[i];
			p.idx = i;
			if(hs[i].pin==1){
				$(p).addClass("pinned");
				p.title = "取消固定";
				$(p).bind('click',function(e){
					cef.plugin.his_unpin(e.currentTarget.idx);
					$(e.currentTarget).attr("title",'固定在此显示');
					$(e.currentTarget).removeClass("pinned");
				});
			}else{
				p.title = "固定在此显示";
				$(p).bind('click',function(e){
					cef.plugin.his_pin(e.currentTarget.idx);
					$(e.currentTarget).attr("title",'取消固定');
					$(e.currentTarget).addClass("pinned");
				});
			}
		}
		$(".edit-bar .remove").bind('click',function(e){
			p = e.currentTarget;
			i = p.getAttribute("idx") * 1;
			cef.plugin.his_del(i);
			$(p).parents('.history_file').hide();
			refresh();
		});
		if(hs[0].name=="" && hs[1].name=="" ){
			$("#dialog-welcome").dialog({modal: false, width:600, close: function(event, ui) { $("#search").focus(); }});
			$("#search").focus();
		}
		$("#loading").css("visibility","hidden");
	}
	function load_history(){
		$("#loading").css("visibility","visible");
		history_parse_timeout = setTimeout(history_parse,1);
	}
	function return_history(){
		$("#loading").css("visibility","visible");
		$("#tabs").fadeOut(300);
		$("#historys").show();
		load_history();
		$("#search").val("");
		$("#search").focus();
	}
	var his_start=true;
	function history_switch(){
		if(his_start){
			history_parse(num_per_page);
			his_start = false;
		}else{
			history_parse(0);
			his_start = true;
		}
	}
	function refresh(){
		if($("#search").val()!="") searchf($("#search").val()); 
		else load_history();
	}
	function parse_init_dir(ss){
			if(ss.charAt(0)=='"'){
			var findex = ss.indexOf('"',1);
			if(findex==-1) return '';
			ss = ss.substring(findex+1, ss.length);
		}else{
			var findex = ss.indexOf(' ',1);
			if(findex==-1) return '';
			ss = ss.substring(findex+1, ss.length);
		}
		ss = trim(ss);
		if(ss.charAt(0)=='"'){
			var findex = ss.indexOf('"',1);
			if(findex==-1) return '';
			ss = ss.substring(1, findex);
		}else{
			var findex = ss.indexOf(' ',1);
			if(findex==-1) return '';
			ss = ss.substring(1, findex);
		}
		return ss;
	}
	function init_dir(ss){ //Called from exe
		var realss = parse_init_dir(ss);
		if(realss.length>=1){
			clearTimeout(history_timeout);
			search_dir(realss);
		}else{
			history_timeout = setTimeout(load_history,50);
		}
		setTimeout(check_upgrade,1);
	}
	function search_dir(ss){
		cef.plugin.dire = ss;
		$("#dir").html(ss);
		var dir_show = ss;
		if(ss.length>38) dir_show=ss.substring(0, 18)+"..."+ss.substring(ss.length-18,ss.length);
		dir_show = "在 \""+dir_show+"\" 下搜索:";
		$("#dir2").val(dir_show);
		$("#dir2").attr("title",ss);
		$("#dir2").css("display","inline");
		$("#dir2").attr("size",dir_show.length+chsum(dir_show)/2);
		var s_width=720-$("#dir2").width();
		$("#search").css("width",s_width+"px");
		$("#search").css("border-left","0px")
		$("#dir2").css("border-right","0px")
		$("#search").addClass("dir_search");
		if($("#search").val()==""){
			$("#search")[0].value = "*";
			$("#search")[0].select();
			searchf("*");
		}else{
			searchf($("#search").val()); 
		}
	}
	function remove_dir(){
		cef.plugin.dire = '';
		$("#dir").html('');
		$("#dir2").css("display","none");
		$("#search").css("width","720px");
		$("#search").css("border-left","1px")
		$("#search").removeClass("dir_search");
	}
	function chsum(chars){
		var sum = 0; 
		for (var i=0; i<chars.length; i++) { 
		   var c = chars.charCodeAt(i); 
		   if ((c >= 0x0001 && c <= 0x007e) || (0xff60<=c && c<=0xff9f)) { 
			 //sum++; 
		   }else {     
		     sum++; 
		   } 
		}
		return sum;
	}
	function init_event(){
		$("#case0").bind('click',function(event,t){
			cef.plugin.caze = false;
			searchf($("#search").val());
		});
		$("#case1").bind('click',function(event,t){
			cef.plugin.caze = true;
			searchf($("#search").val());
		});
		$(document).bind("contextmenu", function(e){
			return false;
		});
		$("#tabs ul li a").bind('click',function(e){
			$(".subnav").hide();
			$("#"+e.target.id+"div").show();
		});
		$("#tabs ul li a img").bind('click',function(e){
			var id = e.target.parentNode.id;
			$(".subnav").hide();
			$("#"+id+"div").show();
			console.log($("#"+id+"div"));
		});
		$(window).bind('scroll',function(e){
			if(reachBottom()) grid_add_files();
		});
		$('#tabs').tabs();
		scan_img();
		if(get_offline_dbs().length==0){
			$(".online_offline").hide();
		}
	}
	$(function() {
		if(cef.plugin.caze){
			$("#case1").click();
		}else{
			$("#case0").click();
		}
		$("#online_bt").click();
		$("#search").focus();
		$("#search").bind('keyup',search_if_change);
		$("#home_bt").bind('click',function(){return_history()});
		$("#refresh_bt").bind('click',function(){refresh()});
		$("#zoomin_bt").bind('click',function(){cef.gigaso.zoom_in()});
		$("#zoomout_bt").bind('click',function(){cef.gigaso.zoom_out()});
		$("#index_st_bt").bind('click',function(){show_index_status()});
		$("#hotkey_bt").bind('click',function(){show_hotkey()});
		$("#export_bt").bind('click',function(){show_export()});
		$("#upgrade_bt").bind('click',function(){check_upgrade(true)});
		$("#largefile_bt").bind('click',function(){$('#search')[0].value='*'; searchf('*');orderby(2,true);});
		$("#offline_bt").bind('click',function(){offline_db()});
		$("#online_bt").bind('click',function(){online_db()});
		$("#dir_bt").bind('click',function(){search_dir(cef.gigaso.select_dir())});
		$("#his_bt").bind('click',function(){history_switch()});
		$("#set_hk_bt").bind('click',function(){set_hotkey()});
		$("#do_exp_bt").bind('click',function(){do_export()});
		$("#do_up_bt").bind('click',function(){do_upgrade()});
		$("#up_later_bt").bind('click',function(){$("#dialog-upgrade").dialog("close");});
		set_orderby_value(0,0);
		setTimeout(init_event,1);
	});
