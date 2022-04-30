#define ICON_STYLE			"style=\"position:fixed;top:%ipx;right:%ipx;max-height:176px;z-index:-1;display:none\" onerror=\"this.style.display='none';\""

static u16 _LINELEN = LINELEN;
static u16 _MAX_PATH_LEN = MAX_PATH_LEN;
static u16 _MAX_LINE_LEN = MAX_LINE_LEN;

#define _1GB_	0x40000000ULL
#define _48GB_	0xC00000000ULL

#define TABLE_ITEM_PREFIX  "<tr><td><a class=\""
#define TABLE_ITEM_SUFIX   "</tr>"

#define TABLE_ITEM_SIZE  28  // strlen(TABLE_ITEM_PREFIX + TABLE_ITEM_SUFIX) = (18 + 10)

#define FILE_MGR_KEY_LEN	10

static const char *set_file_type(const char *path, const char *filename)
{
	if(!wm_icons_exists) return "";

	const char *ext = get_ext(filename);

	if(islike(ext, ".ntfs[PSX") || strstr(path, "/PSX"))
		return " psx";
	#ifdef MOUNT_ROMS
	else if(strstr(path, "/ROMS"))
		return " rom";
	#endif
	else if(is_BIN_ENC(filename) || islike(ext, ".ntfs[PS2") || strstr(path, "/PS2"))
		return " ps2";
	else if(islike(ext, ".ntfs[PSP") || strstr(path, "/PSP"))
		return " psp";
	else if(islike(ext, ".ntfs[DVD") || strstr(path, "/DVD"))
		return " dvd";
	else
		return " ps3";
}

static int add_list_entry(char *param, int plen, char *tempstr, bool is_dir, char *ename, char *templn, char *name, char *fsize, CellRtcDateTime rDate, unsigned long long sz, char *sf, u8 is_net, u8 show_icon0, u8 is_ps3_http, u8 skip_cmd, u8 sort_by, char *action, bool is_ntfs)
{
	bool is_root = (plen < 4);
	if(is_root) sz = 0, is_dir = true; // force folders in root -> fix: host_root, app_home

	unsigned long long sbytes = sz; u16 flen, slen;

	const u16 maxlen = MAX_PATH_LEN - 1;

	//////////////////
	// build labels //
	//////////////////

	const char *ft = "";

	if(!is_dir)
	{
		if(sz < 10240)	{strcpy(sf, STR_BYTE);} else
		if(sz < _2MB_)	{strcpy(sf, STR_KILOBYTE); sz >>= 10;} else
		if(sz < _48GB_) {strcpy(sf, STR_MEGABYTE); sz >>= 20;} else
						{strcpy(sf, STR_GIGABYTE); sz >>= 30;}
	}

	// encode file name for html
	if(is_net)
		flen = htmlenc(tempstr, name, 1);
	else
		flen = strlen(name);

	const char *ext = get_ext(name); *fsize = NULL;

	char is_param_sfo = '>';

	#ifndef LITE_EDITION
	//////////////////////////////////////////
	// show title & title ID from PARAM.SFO //
	//////////////////////////////////////////

	if( !is_dir && IS(name, "PARAM.SFO") )
	{
		char titleid[10], app_ver[8], title[128]; snprintf(title, 127, "%s", templn);

		//get title & app app_ver from PARAM.SFO
		getTitleID(templn, app_ver, GET_VERSION);
		getTitleID(title, titleid, GET_TITLE_AND_ID); get_flag(title, " ["); get_flag(title, "\n");
		get_cover_from_name(tempstr, get_filename(param) + 1, titleid); // get title id from path (if title ID was not found in PARAM.SFO)
		if(*app_ver >= '0') {strcat(title, " v"); strcat(title, app_ver);}
		sprintf(tempstr, "%s%s", HDD0_GAME_DIR, titleid); bool has_updates_dir = file_exists(tempstr);

		sprintf(tempstr, " title=\"%s\">%s</a>", title, name); snprintf(name, maxlen, "%s", tempstr);

		is_param_sfo = ' '; ft = " cfg";

		// show title & link to patches folder
		if(has_updates_dir)
			snprintf(fsize, maxlen, HTML_URL2, HDD0_GAME_DIR, titleid, title);
		else
			snprintf(fsize, maxlen, "%s", title);

		// show title id & link to updates
		if(*titleid && !islike(titleid, "NPWR"))
			sprintf(tempstr, "<div class='sfo'>%s [<a href=\"%s/%s/%s-ver.xml\">%s</a>]</div>", fsize, "https://a0.ww.np.dl.playstation.net/tpl/np", titleid, titleid, titleid);
		else
			sprintf(tempstr, "<div class='sfo'>%s</div>", fsize);

		#ifdef VIEW_PARAM_SFO
		snprintf(fsize, maxlen, "<a href=\"%s%s%s\">%'llu %s</a>%s", "/view.ps3", templn, "", sz, sf, tempstr);
		#else
		snprintf(fsize, maxlen, "%'llu %s%s", sz, sf, tempstr);
		#endif

		#ifdef FIX_GAME
		if(has_updates_dir) snprintf(fsize, maxlen, "<a href=\"%s%s%s\">%'llu %s</a>%s", "/fixgame.ps3", HDD0_GAME_DIR, titleid, sz, sf, tempstr);
		#endif
	}
	#endif

	/////////////////////////
	// encode url for html //
	/////////////////////////

	if(urlenc(tempstr, templn)) {tempstr[_MAX_LINE_LEN] = '\0'; sprintf(templn, "%s", tempstr);}

	// is image?
	u8 show_img = (!is_dir && (_IS(ext, ".png") || _IS(ext, ".jpg") || _IS(ext, ".bmp")));

	char sclass = ' ', dclass = 'w'; // file class

	///////////////////////
	// build size column //
	///////////////////////

	if(is_dir)
	{
		dclass = 'd'; // dir class
		sbytes = 0;

		if(*name == '.')
		{
			snprintf(fsize, maxlen, HTML_URL2, "/stat.ps3", param, HTML_DIR);
		}
		else if(is_root)
		{
			bool show_play = ((flen == 8) && IS(templn, "/dev_bdvd") && IS_ON_XMB);

			if(show_play)
			{
				ft = " ps3";

				if(isDir("/dev_bdvd/PS3_GAME"))
				{
					sprintf(fsize, HTML_URL, "/play.ps3", "&lt;Play>");
				}
				if(file_exists("/dev_bdvd/SYSTEM.CNF"))
				{
					if(wm_icons_exists)
					{
						get_last_game(fsize);
						ft = strcasestr(fsize, "/PSX") ? " psx" : " ps2";
					}
					sprintf(fsize, HTML_URL, "/play.ps3", "&lt;Play>");
				}
				else if(isDir("/dev_bdvd/BDMV"))
				{
					sprintf(fsize, HTML_URL, "/play.ps3", "&lt;BDV>");
				}
				else if(isDir("/dev_bdvd/VIDEO_TS"))
				{
					sprintf(fsize, HTML_URL, "/play.ps3", "&lt;DVD>");
					ft = " dvd";
				}
				else if(isDir("/dev_bdvd/AVCHD"))
				{
					sprintf(fsize, HTML_URL, "/play.ps3", "&lt;AVCHD>");
				}
			}
			else if(IS(templn, "/host_root"))
			{
				strcpy(templn, "/dev_hdd0/packages"); // replace host_root with a shortcut to /dev_hdd0/packages
			}

			if(*fsize) ;

			else if(IS(templn, "/app_home"))
			{
				sprintf(tempstr, "%s/%08i", HDD0_HOME_DIR, xusers()->GetCurrentUserNumber()); snprintf(fsize, maxlen, HTML_URL, tempstr, HTML_DIR);
			}
			else if(IS(templn, "/dev_ps2disc"))
			{
				ft = " ps2";
			}
			else
			#ifdef LITE_EDITION
			{
				if(sys_admin && IS(templn, "/dev_flash"))
					sprintf(fsize, HTML_URL2, "/dev_blind", "?1", HTML_DIR);
				else if(IS(templn, "/dev_blind"))
					sprintf(fsize, HTML_URL2, "/dev_blind", "?0", HTML_DIR);
				else
					snprintf(fsize, maxlen, "<a href=\"/mount.ps3%s\">%s</a>", templn, HTML_DIR);
			}
			#else
			{
				u64 freeSize = 0, devSize = 0;
				#ifdef USE_NTFS
				if(is_ntfs)
					get_ntfs_disk_space(templn + 5, &freeSize, &devSize);
				else
				#endif
				{system_call_3(SC_FS_DISK_FREE, (u64)(u32)(templn), (u64)(u32)&devSize, (u64)(u32)&freeSize);}

				unsigned long long	free_mb    = (unsigned long long)(freeSize>>20),
									free_kb    = (unsigned long long)(freeSize>>10),
									devsize_mb = (unsigned long long)(devSize >>20);

				int d1 = (free_mb    % KB) / 100;
				int d2 = (devsize_mb % KB) / 100;
				char *str_free = STR_MBFREE, *str_mb = STR_MEGABYTE;
				if(devSize >= _1GB_)
				{
					free_mb /= KB, devsize_mb /= KB;
					str_free = STR_GBFREE, str_mb = STR_GIGABYTE;
				}

				if(sys_admin && IS(templn, "/dev_flash"))
					sprintf(tempstr, "%s%s", "/dev_blind", "?1");
				else if(IS(templn, "/dev_blind"))
					sprintf(tempstr, "%s%s", "/dev_blind", "?0");
				#ifdef USE_NTFS
				else if(is_ntfs)
					sprintf(tempstr, "%s%s", "/refresh.ps3", "?prepntfs");
				#endif
				else
					sprintf(tempstr, "%s%s", "/mount.ps3", templn);

				// show graphic bar of device size & free space
				if(devSize > 0)
				{
					sprintf(fsize,  "<div class='bf' style='height:18px;text-align:left;overflow:hidden;'><div class='bu' style='height:18px;width:%i%%'></div><div style='position:relative;top:-%ipx;text-align:right'>"
									"<a href=\"%s\" title=\"%'llu.%i %s (%'llu %s) / %'llu.%i %s (%'llu %s)\">&nbsp; %'8llu.%i %s &nbsp;</a>"
									"</div><	/div>", (int)(100.0f * (float)(devSize - freeSize) / (float)devSize),
									is_ps3_http ? 20 : 18, tempstr,
									free_mb, d1, str_free, freeSize, STR_BYTE,
									devsize_mb, d2, str_mb, devSize, STR_BYTE,
									(freeSize < _2MB_) ? free_kb : free_mb, d1,
									(freeSize < _2MB_) ? STR_KILOBYTE : str_mb);
				}
				else
					snprintf(fsize, maxlen, "<a href=\"%s\">%s</a>", templn, HTML_DIR);
			}
			#endif // #ifdef LITE_EDITION
			if(strstr(fsize, "&lt;")) strcat(fsize, " &nbsp; ");
		}
		#ifdef COPY_PS3
		else if(!is_net && ( IS(name, "VIDEO") || _IS(name, "music") || _IS(name, "covers") || islike(param, HDD0_HOME_DIR) ))
		{
			snprintf(fsize, maxlen, "<a href=\"/copy.ps3%s\" title=\"copy to %s\">%s</a>", islike(templn, param) ? templn + plen : templn, islike(templn, drives[0]) ? drives[usb] : drives[0], HTML_DIR);
		}
		#endif
		#ifdef FIX_GAME
		else if(islike(templn, HDD0_GAME_DIR) || strstr(templn + 10, "/PS3_GAME" ))
		{
			snprintf(fsize, maxlen, HTML_URL2, "/fixgame.ps3", templn, HTML_DIR);
		}
		#endif
		else
			#ifdef PS2_DISC
			snprintf(fsize, maxlen, "<a href=\"/mount%s%s\">%s</a>", strstr(name, "[PS2")?".ps2":".ps3", templn, HTML_DIR);
			#else
			snprintf(fsize, maxlen, "<a href=\"/mount.ps3%s\">%s</a>", templn, HTML_DIR);
			#endif

		// links to home folders
		if((plen == 18 || plen == 26) && islike(templn, "/dev_bdvd/PS3_GAME"))
		{
			*tempstr = NULL;

			if(IS(name, "LICDIR"))
			{
				sprintf(tempstr, "%s/%08i/savedata", HDD0_HOME_DIR, xusers()->GetCurrentUserNumber());
			}
			else if(islike(name, "NPWR"))
			{
				sprintf(tempstr, "%s/%08i/trophy/%s", HDD0_HOME_DIR, xusers()->GetCurrentUserNumber(), name);
			}

			if(isDir(tempstr)) sprintf(fsize, "<a href=\"%s\">%s</a>", tempstr, HTML_DIR);
		}
	}

	else if(*fsize) ;

	else if(skip_cmd)
		sprintf(fsize, "%'llu %s", sz, sf);

	#ifndef LITE_EDITION
	else if( !is_net && (sbytes <= MAX_TEXT_LEN) && ( strcasestr(".txt.ini.log.sfx.xml.cfg.cnf.his.hip.bup.js.css.html.bat|conf", ext) || islike(name, "wm_custom_") || strstr(name, "name") ) )
	{
		snprintf(fsize, maxlen, "<a href=\"/edit.ps3%s\">%'llu %s</a>", templn, sz, sf);
		ft = " cfg";
	}
	#endif

	#ifdef COBRA_ONLY
	else if( (!is_net && strstr(ext, ".ntfs[")) || ((show_icon0 <= 1) && strcasestr(ISO_EXTENSIONS, ext)) )
	{
		if( strcasestr(name, ".iso.") && !is_iso_0(name) && ( !strstr(ext, ".ntfs[") ))
			sprintf(fsize, "<label title=\"%'llu %s\"> %'llu %s</label>", sbytes, STR_BYTE, sz, sf);
		else
			snprintf(fsize, maxlen, "<a href=\"/mount.ps3%s\" title=\"%'llu %s\">%'llu %s</a>", templn, sbytes, STR_BYTE, sz, sf);

		ft = set_file_type(param, ext);
	}
	#endif

	#ifdef PKG_HANDLER
	else if( _IS(ext, ".pkg") || IS(ext, ".p3t"))
	{
		sprintf(fsize, "<a href=\"/install.ps3%s\">%'llu %s</a>", templn, sz, sf);
		ft = " pkg";
	}
	else if( _IS(ext, ".SELF") || strcasestr(ARCHIVE_EXTENSIONS, ext) )
	{
		sprintf(fsize, "<a href=\"/mount.ps3%s\" title=\"%'llu %s\">%'llu %s</a>", templn, sbytes, STR_BYTE, sz, sf);
		ft = " cfg";
	}
	#endif

	#ifdef MOUNT_ROMS
	else if( strstr(templn, "/ROMS") || (!show_icon0 && strcasestr(ROMS_EXTENSIONS, ext)) )
	{
		sprintf(fsize, "<a href=\"/mount.ps3%s\" title=\"%'llu %s\">%'llu %s</a>", templn, sbytes, STR_BYTE, sz, sf);
		ft = show_img ? " pic" : " rom";
	}
	#endif

	#ifdef COPY_PS3
	else if( IS(ext, ".bak") )
		sprintf(fsize, "<a href=\"/rename.ps3%s|\">%'llu %s</a>", templn, sz, sf);
	else if( show_img || _IS(ext, ".mp4") || _IS(ext, ".mkv") || _IS(ext, ".avi") || _IS(ext, ".bik") || _IS(ext, ".mp3") || _IS(ext, ".ac3") || IS(ext, ".AT3") || IS(ext, ".PAM") )
	{
		snprintf(fsize, maxlen, "<a href=\"%s%s\" title=\"%'llu %s copy to %s\">%'llu %s</a>", action, islike(templn, param) ? templn + plen : templn, sbytes, STR_BYTE, islike(templn, drives[0]) ? drives[usb] : drives[0], sz, sf);
		if(!wm_icons_exists) ;
		else if(show_img)
			ft = " pic";
		else if(_IS(ext, ".mp3") || _IS(ext, ".ac3") || IS(ext, ".AT3"))
			ft = " snd";
		else
			ft = " vid";
	}
	else if( IS(ext, ".edat")
			#ifndef PKG_HANDLER
			|| IS(ext,  ".pkg") ||  IS(ext, ".p3t")  || IS(ext, ".PUP")
			#endif
			|| IS(ext,  ".rco") ||  IS(ext, ".qrc")
			|| !memcmp(name, "webftp_server", 13) || !memcmp(name, "coldboot", 8)
			#ifdef SWAP_KERNEL
			|| !memcmp(name, "lv2_kernel", 10)
			#endif
			)
	{
		sprintf(fsize, "<a href=\"%s%s\" title=\"%'llu %s copy to %s\">%'llu %s</a>", action, islike(templn, param) ? templn + plen : templn, sbytes, STR_BYTE, islike(templn, drives[0]) ? drives[usb] : drives[0], sz, sf);
		ft = " pkg";
	}
	#endif //#ifdef COPY_PS3

	#ifdef VIEW_PARAM_SFO
	else if( IS(ext, ".SFO") )
	{
		sprintf(fsize, "<a href=\"/view.ps3%s\">%'llu %s</a>", templn, sz, sf);
		ft = " cfg";
	}
	#endif

	#ifdef LOAD_PRX
	else if(!is_net && ( IS(ext, ".sprx")))
	{
		snprintf(fsize, maxlen, "<a href=\"/loadprx.ps3?slot=6&prx=%s\">%'llu %s</a>", templn, sz, sf);
		ft = " pkg";
	}
	#endif

	else if(sbytes < 10240)
		sprintf(fsize, "%'llu %s", sz, sf);
	else
		sprintf(fsize, "<label title=\"%'llu %s\"> %'llu %s</label>", sbytes, STR_BYTE, sz, sf);

	////////////////////
	// build sort key //
	////////////////////

	if(sort_by == 's')
	{	// convert sbyte to base 255 to avoid nulls that would break strncmp in sort
		memset(ename, 1, FILE_MGR_KEY_LEN -1); u8 index = 6;
		while (sbytes > 0)
		{
			ename[index] = 1 + (sbytes % 255);
			sbytes = sbytes / 255ULL;
			if(index == 0) break; else index--;
		}
	}
	else if(sort_by == 'd')
		snprintf(ename, FILE_MGR_KEY_LEN, "%c%c%c%c%c", ((rDate.year - 1968) % 223) + 0x20, rDate.month+0x20, rDate.day+0x20, rDate.hour+0x20, rDate.minute+0x20);
	else
	{
		//if(*name == '0' && flen == 8 && IS(param, HDD0_HOME_DIR))
		//	snprintf(ename, FILE_MGR_KEY_LEN, "%s", name + 3);
		//else
		snprintf(ename, FILE_MGR_KEY_LEN, "%s         ", name); sclass = dclass;

		if(flen > FILE_MGR_KEY_LEN -2) {char c = name[flen - 1]; if(ISDIGIT(c)) ename[FILE_MGR_KEY_LEN - 2] = c;} // sort isos
	}

	if((plen > 1) && memcmp(templn, param, plen) == 0) sprintf(templn, "%s", templn + plen + 1); // remove path from templn (use relative path)


	//////////////////////
	// build list entry //
	//////////////////////

	const u16 dlen = FILE_MGR_KEY_LEN + 21; // key length + length of date column

	if(wm_icons_exists && *ft) dclass = 'w';

	// -- key
	*tempstr = sclass; memcpy(tempstr + 1, ename, FILE_MGR_KEY_LEN -1);

	// -- name column
	flen = sprintf(tempstr + FILE_MGR_KEY_LEN,
							 "%c%s\" href=\"%s\"%s%c%s</a>",
							 dclass, ft, templn,
							#ifndef LITE_EDITION
							 show_img ? " onmouseover=\"s(this,0);\"" : (is_dir && show_icon0) ? " onmouseover=\"s(this,1);\"" :
							#endif
							"" , is_param_sfo, name);

	// -- size column
	slen =  sprintf(templn, "<td> %s%s",
							fsize, is_root ? "" : " &nbsp; ");

	// -- reduce features if html code is too long
	if((FILE_MGR_KEY_LEN + flen + slen + dlen) >= _LINELEN)
	{
		// -- remove link from size column
		if(is_dir) sprintf(fsize, HTML_DIR); else sprintf(fsize, "%llu %s", sz, sf);

		// -- rebuild size column without link
		slen = sprintf(templn, "<td> %s%s",
								fsize, is_root ? "" : " &nbsp; ");

		// -- rebuild name without link if html code is still too long
		if((FILE_MGR_KEY_LEN + flen + slen + dlen) >= _LINELEN)
		{
			flen = sprintf(tempstr + FILE_MGR_KEY_LEN,
									 "%c%s\"%c%s</a>",
									 dclass, ft, is_param_sfo, name);
		}
	}

	// append size column
	sprintf(tempstr + FILE_MGR_KEY_LEN + flen, "%s", templn);

	// append date column (21 bytes)
	sprintf(tempstr + FILE_MGR_KEY_LEN + flen + slen, "<td>%02i-%s-%04i %02i:%02i",
														rDate.day, smonth[rDate.month-1], rDate.year, rDate.hour, rDate.minute);

	flen += slen + dlen; // size of key + name column + size column + date column

	if(FILE_MGR_KEY_LEN + flen >= _LINELEN) {flen = 0, *tempstr = NULL;} //ignore file if it is still too long

	return flen;
}

static int add_breadcrumb_trail(char *pbuffer, const char *param)
{
	int slen, tlen = 0;

	char swap[MAX_PATH_LEN], templn[MAX_PATH_LEN], url[MAX_PATH_LEN], *slash, *buffer = pbuffer;

	strcpy(templn, param);

	// add links to path
	fast_concat.str = NULL;

	while((slash = strchr(templn + 1, '/')))
	{
		*slash = NULL;
		tlen+=(slash - templn) + 1; //strlen(templn) + 1;

		strcpy(swap, param);
		swap[tlen] = '\0';

		buffer += concat(buffer, "<a class=\"f\" href=\"");
		urlenc(url, swap);
		buffer += concat(buffer, url);

		htmlenc(url, templn, 1);
		sprintf(swap, "\">%s</a>/", templn);
		buffer += concat(buffer, swap);

		strcpy(templn, param + tlen);
	}

	// add link to file or folder
	if(!param[1]) slen = sprintf(swap, "/");
	else if((param[1] != 'n') && not_exists(param)) slen = sprintf(swap, "%s", get_filename(param) + 1);
	else
	{
		tlen = strlen(param) - 4; if(tlen < 0) tlen = 0;

		char label[_MAX_PATH_LEN];
		urlenc(url, param); if(islike(param, "/net")) htmlenc(label, templn, 0); else strcpy(label, templn);

		#ifdef USE_NTFS
		if(is_ntfs_path(param)) slen = sprintf(swap, HTML_URL, WMTMP, label);
		else
		#endif
		slen = sprintf(swap, HTML_URL2,
						(use_open_path || strstr(pbuffer, "To: ") || strstr(pbuffer, "Path: ")) ? "" :
						islike(param + 23, "/trophy/NPWR") ? "/delete.ps3" :
		#ifdef COPY_PS3
						islike(param, HDD0_HOME_DIR) ? "/copy.ps3" :
		#endif
		#ifdef FIX_GAME
						islike(param, HDD0_GAME_DIR) ? "/fixgame.ps3" :
		#endif
		#ifdef PKG_HANDLER
						is_ext(param + tlen, ".pkg") ? "/install.ps3" :
		#endif
						islike(param + 15, "/covers") ? "" : // /dev_hdd0/GAMES/covers
						((isDir(param) ||
						 strcasestr(ISO_EXTENSIONS, param + tlen)) ||
						 strstr(param, "/GAME")  ||
						 strstr(param, ".ntfs[") ||
						 islike(param, "/net") ) ? "/mount.ps3" : "", url, label);
	}

	// add code to buffer
	use_open_path = false;

	concat(buffer, swap);

	return slen + (buffer - pbuffer);
}

static int add_breadcrumb_trail2(char *pbuffer, const char *label, const char *param)
{
	use_open_path = true;
	if(label) {strcat(pbuffer, label); strcat(pbuffer, " ");}
	return add_breadcrumb_trail(pbuffer, param);
}

static bool folder_listing(char *buffer, u32 BUFFER_SIZE_HTML, char *templn, char *param, int conn_s, char *tempstr, char *header, u8 is_ps3_http, s8 sort_by, s8 sort_order, char *file_query)
{
	int fd; t_string sout; _set(&sout, buffer, strlen(buffer));

	CellRtcDateTime rDate;

	if(sys_admin && islike(param, "/dev_blind?"))
	{
		if( param[11] & 1) enable_dev_blind(NO_MSG); else //enable
		if(~param[11] & 1) disable_dev_blind();           //disable

		if( param[11] ) {sprintf(templn, HTML_REDIRECT_TO_URL, "/", HTML_REDIRECT_WAIT); _concat(&sout, templn);}

		_concat2(&sout, "/dev_blind: ", isDir("/dev_blind") ? STR_ENABLED : STR_DISABLED);
		return true;
	}

	normalize_path(param, false);

	absPath(templn, param, "/"); // auto mount /dev_blind & /dev_hdd1

	u8 is_net = (param[1] == 'n'), skip_cmd = 0;

	bool is_ntfs = false;

	int plen = strlen(param);
	bool is_root = (plen < 4);

	#ifdef USE_NTFS
	struct stat bufn;
	DIR_ITER *pdir = NULL;
	char *npath = NULL;

	is_ntfs = is_ntfs_path(param);
	if((is_root && root_check) | is_ntfs) check_ntfs_volumes();

	if(is_ntfs)
	{
		npath = (char*)ntfs_path(param); // /dev_ntfs1v -> ntfs1:/
		pdir = ps3ntfs_opendir(npath);
		if(!pdir) is_ntfs = false;
		cellRtcSetTime_t(&rDate, 0);
		if(!npath[7]) npath[6] = 0;
	}
	#endif

	#if defined(FIX_GAME) || defined(COPY_PS3)
	if(copy_aborted) _concat(&sout, STR_CPYABORT);    //  /copy.ps3$abort
	else
	if(fix_aborted)  _concat(&sout, "Fix aborted!");  //  /fixgame.ps3$abort

	if(copy_aborted | fix_aborted) {_concat(&sout, "<p>"); sys_ppu_thread_usleep(100000); copy_aborted = fix_aborted = false;}
	#endif

	_LINELEN = LINELEN;
	_MAX_PATH_LEN = MAX_PATH_LEN;
	_MAX_LINE_LEN = MAX_LINE_LEN;

	if(is_ntfs || is_net || cellFsOpendir(param, &fd) == CELL_FS_SUCCEEDED)
	{
		if(!extcmp(param, "/exdata", 7)) {_LINELEN = _MAX_LINE_LEN = _MAX_PATH_LEN = 200; skip_cmd = 1;}

		unsigned long long sz = 0, dir_size = 0;
		char ename[16], sf[8];
		char fsize[MAX_PATH_LEN], *swap = fsize;
		u16 idx = 0, dirs = 0, flen; bool is_dir;
		u32 tlen = 0;
		char *sysmem_html = buffer + (webman_config->sman ? _12KB_ : _6KB_);

		typedef struct
		{
			char path[_LINELEN];
		} t_line_entries;

		t_line_entries *line_entry = (t_line_entries *)sysmem_html;
		u16 max_entries = ((BUFFER_SIZE_HTML - _16KB_) / _MAX_LINE_LEN) - 1;

		BUFFER_SIZE_HTML -= _4KB_;

		u8 jb_games = (strstr(param, "/GAMES") || strstr(param, "/GAMEZ"));
		u8 show_icon0 = jb_games ? 1 :
						islike(param, "/dev_hdd0/game") ? 2 :
						islike(param, HDD0_HOME_DIR)    ? 3 : 0;

		char *action = (char*)"/copy.ps3";

		#ifdef VISUALIZERS
		if(islike(param, "/dev_hdd0/tmp/wallpaper")) action = (char*)"/wallpaper.ps3";
		if(islike(param, "/dev_hdd0/tmp/earth"    )) action = (char*)"/earth.ps3";
		if(islike(param, "/dev_hdd0/tmp/canyon"   )) action = (char*)"/canyon.ps3";
		if(islike(param, "/dev_hdd0/tmp/lines"    )) action = (char*)"/lines.ps3";
		if(islike(param, "/dev_hdd0/tmp/coldboot" )) action = (char*)"/coldboot.ps3";
		#endif

		#ifndef LITE_EDITION
		sprintf(templn, "<img id=\"icon\" " ICON_STYLE ">"
						"<script>"
						// show icon of item pointed with mouse
						"function s(o,d){u=o.href;p=u.indexOf('.ps3');if(p>0)u=u.substring(p+4);if(d){p=u.indexOf('/PS3_');if(p<0)p=u.indexOf('/USRDIR');if(p>0)u=u.substring(0,p);u+='%s/ICON0.PNG';}icon.src=u;icon.style.display='block';}"
						"</script>", webman_config->sman ? 98 : 118, webman_config->sman ? 25 : 10, (jb_games ? "/PS3_GAME" : "")); _concat(&sout, templn);
		#endif

		// breadcrumb trail //
		sout.size += add_breadcrumb_trail(buffer, param); if(param[10] != ':') _concat(&sout, ":");

		if((param[7] == 'v' || param[1] == 'a') && IS_ON_XMB && (isDir("/dev_bdvd/PS3_GAME") || file_exists("/dev_bdvd/SYSTEM.CNF") || isDir("/dev_bdvd/BDMV") || isDir("/dev_bdvd/VIDEO_TS") || isDir("/dev_bdvd/AVCHD")))
			_concat(&sout, " [<a href=\"/play.ps3\">Play</a>]<br>");
		else
			_concat(&sout, "<br>");

		#ifdef COPY_PS3
		if(cp_mode) {sprintf(tempstr, "<font size=2><a href=\"/paste.ps3%s\">&#128203;</a> ", is_net ? "/dev_hdd0/packages" : param); add_breadcrumb_trail(tempstr, cp_path); _concat(&sout, tempstr); _concat(&sout, "</font><p>"); }

		usb = get_default_usb_drive(0);
		#endif

		_concat(&sout,  "<style>.sfo{position:absolute;top:308px;right:10px;font-size:14px}td+td{text-align:right}</style>"
						"<table id=\"files\" class=\"propfont\">");
/*
		if(file_exists("/dev_hdd0/xmlhost/game_plugin/sort.js"))
			_concat(&sout, "<script src=\"/dev_hdd0/xmlhost/game_plugin/sort.js\"></script>"
									 "<thead><tr><th align=left>Name</th><th>Size</th><th>Date</th></tr></thead>");
		else
*/
			_concat(&sout, "<tr><td colspan=3><col width=\"220\"><col width=\"98\">");

		tlen = sout.size;

		#ifdef NET_SUPPORT
		if(is_net)
		{
			int ns = FAILED, abort_connection = 0; char netid = param[4]; netiso_svrid = (netid & 0x0F);

			if(netid >= '0' && netid <= '4') ns = connect_to_remote_server(netid);

			if(ns >= 0)
			{
				char *netpath = param + 5;

				normalize_path(netpath, true);

				if(open_remote_dir(ns, netpath, &abort_connection, false) >= 0)
				{
					strcpy(templn, param); normalize_path(templn, false); remove_filename(templn);
					if(strlen(templn) < 5) strcpy(templn, "/");

					urlenc(swap, templn);
					flen = sprintf(line_entry[idx].path,  "!         " // <-- size should be = FILE_MGR_KEY_LEN
														  "d\" href=\"%s\">..</a>"
														  "<td> " HTML_URL "%s"
														, swap, swap, HTML_DIR, HTML_ENTRY_DATE);

					if(flen >= _MAX_LINE_LEN) //path is too long
					{
						sclose(&ns);
						return false;
					}

					idx++, dirs++;
					tlen += flen;

					sys_addr_t data = 0;
					netiso_read_dir_result_data *dir_items = NULL;
					int v3_entries = read_remote_dir(ns, &data, &abort_connection);
					if(data)
					{
						dir_items = (netiso_read_dir_result_data*)data;

						for(int n = 0; n < v3_entries; n++)
						{
							if(dir_items[n].name[0] == '.' && dir_items[n].name[1] == 0) continue;
							if(tlen > BUFFER_SIZE_HTML) break;
							if(idx >= (max_entries-3)) break;

							if(*file_query && (strcasestr(dir_items[n].name, file_query) == NULL)) continue;

							if(param[1] == 0)
								flen = sprintf(templn, "/%s", dir_items[n].name);
							else if(dir_items[n].name[0] == '/')
								flen = sprintf(templn, "%.5s%s", param, dir_items[n].name);
							else
								flen = sprintf(templn, "%s%s", param, dir_items[n].name);

							 normalize_path(templn, false);

							cellRtcSetTime_t(&rDate, dir_items[n].mtime);

							sz = (unsigned long long)dir_items[n].file_size; dir_size += sz;

							is_dir = dir_items[n].is_directory; if(is_dir) dirs++;

							flen = add_list_entry(param, plen, tempstr, is_dir, ename, templn, dir_items[n].name, fsize, rDate, sz, sf, true, show_icon0, is_ps3_http, skip_cmd, sort_by, action, is_ntfs);

							if((flen == 0) || (flen >= _MAX_LINE_LEN)) continue; //ignore lines too long
							memcpy(line_entry[idx].path, tempstr, FILE_MGR_KEY_LEN + flen + 1); idx++;
							tlen += (flen + TABLE_ITEM_SIZE);

							if(!working) break;
						}
						sys_memory_free(data);
					}
				}
				else //may be a file
				{
					int is_directory = 0;
					s64 file_size;
					u64 mtime, ctime, atime;
					if(remote_stat(ns, netpath, &is_directory, &file_size, &mtime, &ctime, &atime, &abort_connection) == CELL_OK)
					{
						if(file_size && !is_directory)
						{
							if(open_remote_file(ns, netpath, &abort_connection) > 0)
							{
								size_t header_len = prepare_header(header, param, 1);
								header_len += sprintf(header + header_len, "Content-Length: %llu\r\n\r\n", (unsigned long long)file_size);

								send(conn_s, header, header_len, 0);

								u32 bytes_read; s64 offset = 0;
								while(offset < file_size)
								{
									bytes_read = read_remote_file(ns, (char*)buffer, offset, _64KB_, &abort_connection);
									if(bytes_read)
									{
										if(send(conn_s, buffer, bytes_read, 0) < 0) break;
									}
									offset += bytes_read;
									if(bytes_read < _64KB_ || offset >= file_size) break;
								}
								open_remote_file(ns, "/CLOSEFILE", &abort_connection);
								sclose(&ns);
								sclose(&conn_s);

								return false; // net file
							}
						}
					}
				}
				sclose(&ns);
			}
		}
		else
		#endif // #ifdef NET_SUPPORT
		{
			bool is_bdvd = islike(param, "/dev_bdvd");
			CellFsDirectoryEntry entry; u32 read_f;
			CellFsDirent entry_s; u64 read_e; // list root folder using the slower readdir
			char *entry_name = (is_root | is_bdvd) ? entry_s.d_name : entry.entry_name.d_name;

			#ifdef USE_NTFS
			if(is_ntfs && !npath[6])
			{
				flen = sprintf(line_entry[idx].path,  "!         " // <-- size should be = FILE_MGR_KEY_LEN
													  "d\" href=\"%s\">..</a>"
													  "<td> " HTML_URL "%s"
													, "/", "/", HTML_DIR, HTML_ENTRY_DATE);
				idx++, dirs++;
				tlen += flen;
			}
			#endif

			while(working)
			{
				#ifdef USE_NTFS
				if(is_ntfs)
				{
					if(ps3ntfs_dirnext(pdir, entry_name, &bufn)) break;
					if(entry_name[0] == '$' && npath[6] == 0) continue;

					entry.attribute.st_mode = bufn.st_mode, entry.attribute.st_size = bufn.st_size, entry.attribute.st_mtime = bufn.st_mtime;
				}
				else
				#endif
				if(is_root | is_bdvd) {if((cellFsReaddir(fd, &entry_s, &read_e) != CELL_FS_SUCCEEDED) || (read_e == 0)) break;}
				else
				if(cellFsGetDirectoryEntries(fd, &entry, sizeof(entry), &read_f) || !read_f) break;

				if(entry_name[0] == '.' && entry_name[1] == 0) continue;
				if(tlen > BUFFER_SIZE_HTML) break;
				if(idx >= (max_entries-3)) break;

				if(*file_query && (strcasestr(entry_name, file_query) == NULL)) continue;

				#ifdef USE_NTFS
				// use host_root to expand all /dev_ntfs entries in root
				bool is_host = is_root && ((mountCount > 0) && IS(entry_name, "host_root") && mounts);

				u8 ntmp = 1;
				if(is_host) ntmp = mountCount + 1;

				for (u8 u = 0; u < ntmp; u++)
				{
					if(u) {sprintf(entry_name, "dev_%s", mounts[u-1].name); is_ntfs = true;}
				#endif
					if(is_root)
					{
						flen = sprintf(templn, "/%s", entry_name);
					}
					else
					{
						flen = sprintf(templn, "%s/%s", param, entry_name);
					}
					if(templn[flen - 1] == '/') templn[flen--] = '\0';

					if(is_root | is_bdvd)
					{
						struct CellFsStat buf;
						cellFsStat(templn, &buf);
						entry.attribute.st_mode  = buf.st_mode;
						entry.attribute.st_size  = buf.st_size;
						entry.attribute.st_mtime = buf.st_mtime;
					}

					cellRtcSetTime_t(&rDate, entry.attribute.st_mtime);

					sz = (unsigned long long)entry.attribute.st_size; dir_size += sz;

					is_dir = (entry.attribute.st_mode & S_IFDIR); if(is_dir) dirs++;

					flen = add_list_entry(param, plen, tempstr, is_dir, ename, templn, entry_name, fsize, rDate, sz, sf, false, show_icon0, is_ps3_http, skip_cmd, sort_by, action, is_ntfs);

					if((flen == 0) || (flen >= _MAX_LINE_LEN)) continue; //ignore lines too long
					memcpy(line_entry[idx].path, tempstr, FILE_MGR_KEY_LEN + flen + 1); idx++;
					tlen += (flen + TABLE_ITEM_SIZE);

					if(!working) break;

				#ifdef USE_NTFS
				}
				if(is_root) is_ntfs = false;
				#endif
			}

			#ifdef USE_NTFS
			if(is_ntfs)
			{
				if(pdir) ps3ntfs_dirclose(pdir);
			}
			else
			#endif
			cellFsClosedir(fd);
		}

		/////////////////////////////
		// add net entries to root //
		/////////////////////////////

		#ifdef NET_SUPPORT
		if(is_root)
		{
			for(u8 n = 0; n < 5; n++)
			{
				if(is_netsrv_enabled(n))
				{
					sprintf(line_entry[idx].path, "dnet%i     " // <-- size should be = FILE_MGR_KEY_LEN
												  "d\" href=\"/net%i\">net%i (%s:%i)</a>"
												  "<td> <a href=\"/mount.ps3/net%i\">%s</a>%s"
												  , n, n, n, webman_config->neth[n], webman_config->netp[n], n, HTML_DIR, HTML_ENTRY_DATE); idx++;
				}
			}
		}
		#endif

		///////////////////////
		// sort list entries //
		///////////////////////

		if(idx)
		{   // sort html file entries
			u16 n, m;
			t_line_entries swap;
			for(n = 0; n < (idx - 1); n++)
				for(m = (n + 1); m < idx; m++)
					if(sort_order * strncmp(line_entry[n].path, line_entry[m].path, FILE_MGR_KEY_LEN) > 0)
					{
						swap = line_entry[n];
						line_entry[n] = line_entry[m];
						line_entry[m] = swap;
					}
		}

		//////////////////////
		// add list entries //
		//////////////////////

		for(u16 m = 0; m < idx; m++)
		{
			if(tlen > BUFFER_SIZE_HTML) break;

			_concat(&sout, TABLE_ITEM_PREFIX);
			_concat(&sout, (line_entry[m].path) + FILE_MGR_KEY_LEN);
			_concat(&sout, TABLE_ITEM_SUFIX);
		}

		_concat(&sout, "</table>");

		//////////////////
		// build footer //
		//////////////////

		if(!is_root)
		{
			///////////
			#ifdef COBRA_ONLY
			unsigned int real_disctype, iso_disctype, effective_disctype = 1;
			#endif

			if(!is_ps3_http)
			{
				bool show_icon = false;
				if(is_net && jb_games)
				{
					replace_char(param + 12, '/', 0);
					sprintf(templn, "%s/PS3_GAME/ICON0.PNG", param);
					show_icon = true;
				}

				if(!show_icon)
				{
					sprintf(templn, "%s/ICON0.PNG", param); show_icon = file_exists(templn);                    // current folder
					if(!show_icon) sprintf(templn, "%s/ICON2.PNG", param); show_icon = file_exists(templn);     // ps3_extra folder
					if(!show_icon)
					{
						replace_char(param + 18, '/', 0);
						sprintf(templn, "%s/PS3_GAME/ICON0.PNG", param); show_icon = file_exists(templn); // dev_bdvd or jb folder
						if(!show_icon) sprintf(templn, "%s/ICON0.PNG", param); show_icon = file_exists(templn); // game dir
					}
				}

				if(!show_icon && islike(param, "/dev_bdvd"))
				{
					enum icon_type dt = iPS3;

					#ifdef COBRA_ONLY
					cobra_get_disc_type(&real_disctype, &effective_disctype, &iso_disctype);
					if(iso_disctype == DISC_TYPE_PSX_CD) dt = iPSX; else
					if(iso_disctype == DISC_TYPE_PS2_DVD
					|| iso_disctype == DISC_TYPE_PS2_CD) dt = iPS2; else
					if(iso_disctype == DISC_TYPE_DVD)    dt = iDVD;
					#endif

					sprintf(templn, "%s", wm_icons[dt]); show_icon = true;
				}

				for(u16 m = idx; m < 10; m++) _concat(&sout, "<br>");

				if(show_icon || show_icon0)
					{urlenc(swap, templn); sprintf(templn, "<script>icon.src=\"%s\";icon.style.display='block';</script>", swap); _concat(&sout, templn);}
			}
			///////////

			#ifdef COPY_PS3
			// add fm.js script
			if(file_exists(FM_SCRIPT_JS))
			{
				sprintf(templn, SCRIPT_SRC_FMT, FM_SCRIPT_JS); _concat(&sout, templn);
			}
			#endif // #ifdef COPY_PS3

			*tempstr = NULL;

			#ifdef COBRA_ONLY
			// show last mounted game
			if(effective_disctype != DISC_TYPE_NONE && IS(param, "/dev_bdvd"))
			{
				// get last game path
				get_last_game(templn);

				if(*templn == '/') {sprintf(tempstr, HTML_SHOW_LAST_GAME); add_breadcrumb_trail(tempstr, templn); strcat(tempstr, HTML_SHOW_LAST_GAME_END);}
			}
			#endif

			///////////
			replace_char(param + 1, '/', 0);

			sprintf(templn, "<hr>"
							"<b>" HTML_URL "%c", param, param, (param[10] == ':') ? 0 : ':');

			_concat(&sout, templn);

			if(param[1] != 'n') // not /net
			{
				free_size(param, templn);
				_concat(&sout, templn);
			}

			// summary
			sprintf(templn, "</b> &nbsp; <font color=\"#C0C0C0\">%'i Dir(s) %'d %s %'d %s</font>%s",
							MAX(dirs - 1, 0), (idx - dirs), STR_FILES,
							dir_size<(_1MB_) ? (int)(dir_size>>10):(int)(dir_size>>20),
							dir_size<(_1MB_) ? STR_KILOBYTE:STR_MEGABYTE, tempstr);

			_concat(&sout, templn);
			///////////
		}
		else
		{
			_concat(&sout,	HTML_BLU_SEPARATOR);

			#ifndef LITE_EDITION
			_concat2(&sout, webman_config->sman ? "<a class='tg' " : "<a ",
							"onclick=\"o=lg.style,o.display=(o.display=='none')?'block':'none';\" "
							"style=\"cursor:pointer;\"> ");
			#endif

			_concat2(&sout, WM_APPNAME, " - Simple Web Server" EDITION "</a>");

			#ifndef LITE_EDITION
			_concat(&sout, "<div id=\"lg\" style=\"display:none\">");

			_lastgames lastgames;

			if(read_file(LAST_GAMES_BIN, (char*)&lastgames, sizeof(_lastgames), 0))
			{
				u8 n, m; char *lastgame;

				// verify that last games exist
				for(n = 0; n < MAX_LAST_GAMES; n++)
				{
					lastgame = lastgames.game[n].path;
					if(!islike(lastgame, "/net") && not_exists(lastgame)) *lastgame = NULL;
				}

				// sort last games
				t_path_entries swap;
				for(n = 0; n < (MAX_LAST_GAMES - 1); n++)
					for(m = (n + 1); m < MAX_LAST_GAMES; m++)
						if(*lastgames.game[n].path == '/' && *lastgames.game[m].path == '/' && (strcasecmp(get_filename(lastgames.game[n].path), get_filename(lastgames.game[m].path)) > 0))
						{
							swap = lastgames.game[n];
							lastgames.game[n] = lastgames.game[m];
							lastgames.game[m] = swap;
						}

				// show last games
				const char *ft;
				for(n = 0; n < MAX_LAST_GAMES; n++)
				{
					lastgame = lastgames.game[n].path;
					if(*lastgame)
					{
						const char *name = get_filename(lastgame); if(!name) name = lastgame; else name++;

						ft = set_file_type(lastgame, name);

						sprintf(tempstr, "<a class=\"%c%s\" href=\"/mount.ps3%s\">%s</a><br>",
								(isDir(lastgame) || strstr(lastgame, "/GAME")) ? 'd' : 'w', ft, lastgame, name);
						_concat(&sout, tempstr);
					}
				}
			}
			_concat(&sout, "</div>");
			#endif // #ifndef LITE_EDITION
		}
	}
	return true;
}
