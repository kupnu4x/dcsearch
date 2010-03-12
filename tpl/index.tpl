<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html;charset=utf-8" />
<meta name="Description" content="" />
<title></title>
<style type="text/css">
    body, div{
        margin:0;
        padding:0;
    }
    img{
        border:0;
    }
    #top{
        background-color: #000;
        height:50px;
        padding-top:5px;
    }
    #logo{
        font-family: Arial;
        font-size: 38px;
        color:#fff;
        margin:25px 0 0 25px;
    }
    #logo a{
        text-decoration: none;
        color:#fff;
    }
    #top form{
        display: inline;
        margin-left:25px;
        position:relative;
        top:-5px;
    }
    #q{
        width:500px;
    }
    #filter{
        background:#e0e0e0;
        border-bottom:1px solid #808080;
        font-family:Arial;
        font-size:13px;
        line-height:28px;
        color:#505050;
        padding-left:25px
    }
    #filter form{
        margin:0;
    }
    #filter input{
        text-align: center;
    }
    #results{
        font-family: Arial;
        font-size: 12px;
        line-height: 24px;
        padding:10px 10px 10px 25px;
    }
    .result{
        margin: 10px 0 10px 0;
        padding : 0 0 0 42px;
        line-height:18px;
    }
    .result img.type{
        float:left;
        margin: 2px -32px 0 -42px;
    }
    .result img.magnet{
        vertical-align: top;
        position:relative;
        top:1px;
    }
    #footer{
        border-top:1px solid #000;
    }
    #pagination{
        float:left;
        font-family: Arial;
        font-size:12px;
        padding:10px 10px 10px 25px;
    }
    #pagination a{
        color:#0000ee;
    }
    .ctrlhint{
        color:#999;
        font-size: 13px;
    }
    #copyright{
        float:right;
        font-family: Arial;
        font-size: 10px;
        margin:10px 10px 0 0;
        text-align:right;
    }
    #copyright img{
        vertical-align: bottom;
    }
    .clear{
        clear:both;
        margin:-10px;
    }
</style>
{{if pagination}}
    {{if prevlink}}<link rel="prev" href="{{prevlink}}" id="prevlink" />{{end}}
    {{if nextlink}}<link rel="next" href="{{nextlink}}" id="nextlink" />{{end}}
    <script type="text/javascript">
        function ctrlMove(e){
            var key = e.keyCode?e.keyCode:(e.which?e.which:null);
            var link;
            switch(key){
                case 0x25:
                    link = document.getElementById ('prevlink');
                    break;
                case 0x27:
                    link = document.getElementById ('nextlink');
                    break;
            }
            if(link && link.href) document.location.href = link.href;
        }
        document.onkeydown = ctrlMove;
    </script>
{{end}}
</head>
<body>
    <div id="top">
        <span id="logo"><a href="?">DCSearch</a></span>
        {{unless hide_search_form}}
        <form method="get">
            <input type="text" name="q" id="q" value="{{query}}" />
            <input type="submit" value="search" />
        </form>
        {{end}}
    </div>
    {{if filter_last}}
    <div id="filter">
        <form method="get">
        {{begin filter_last}}
            Show only
            <select name="cat">
                {{begin categories}}
                <option value="{{key}}" {{if selected}}selected {{end}}>{{value}}</option>
                {{end}}
            </select>
            by last
            <input type="text" name="d" value="{{days}}" size="2" maxlength="2" /> days
        {{end}}
        <input type="submit" value="Go" />
        </form>
    </div>
    {{end}}
    <div id="results">
        {{if viewlast_categories}}
            View latest:<br/>
            {{begin viewlast_categories}}
                <a href="?cat={{key}}&d=7">{{value}}</a><br/>
            {{end}}
        {{end}}

        {{if nofound}}
            Sorry, no matches found
        {{end}}
        
        {{begin results}}
            <div class="result">
            {{if rowtype_tth}}
                <img src="img/tth.png" class="type" alt="TTH" /> //{{nick}}/{{fullpath}}/{{name}}.{{extension}}<br/>
                <small>Size: {{size}}, TTH: <b>{{tth}}</b> <a href="{{magnet}}"><img src="img/magnet.png" class="magnet" alt="magnet link" /></a> </small><br/>
                <small style="color:#999">In index since {{starttime}}, Last seed {{lasttime}}</small>
            {{end}}
            {{if rowtype_dir}}
                <img src="img/folder.png" class="type" alt="dir" /> //{{nick}}/{{fullpath}}/<b>{{name}}</b>/<br/>
                <small><i>directory</i></small>
            {{end}}
            {{if rowtype_file}}
                <img src="img/{{type}}.png" class="type" alt="file" /> //{{nick}}/{{fullpath}}/<b>{{name}}.{{extension}}</b><br/>
                <small>Size: {{size}}, TTH: {{tth}} <a href="{{magnet}}"><img src="img/magnet.png" class="magnet" alt="magnet link" /></a> </small><br/>
                <small style="color:#999">In index since {{starttime}}, Last seed {{lasttime}}</small>
            {{end}}
            <br class="clear"/>
            </div>
        {{end}}
    </div>
    {{if results}}
    <div id="footer">
        {{if pagination}}
        <div id="pagination">
            {{begin pagination_search}}
                {{unless selected}}<a href="?{{unless $_first}}p={{$_num}}&{{end}}q={{query}}">{{$_num}}</a>{{end}}
                {{if selected}}<b>{{$_num}}</b>{{end}}
                {{unless $_last}}, {{end}}
            {{end}}

            {{begin pagination_viewlast}}
                {{unless selected}}<a href="?{{unless $_first}}p={{$_num}}&{{end}}cat={{category}}&d={{days}}">{{$_num}}</a>{{end}}
                {{if selected}}<b>{{$_num}}</b>{{end}}
                {{unless $_last}}, {{end}}
            {{end}}
            <br class="clear"/>
            <span class="ctrlhint">
                {{if prevlink}}<a href="{{prevlink}}">&larr;</a>{{end}}
                {{unless prevlink}}&larr;{{end}}
                ctrl
                {{if nextlink}}<a href="{{nextlink}}">&rarr;</a>{{end}}
                {{unless nextlink}}&rarr;{{end}}
            </span>
        </div>
        {{end}}
        {{if powered_sphinx}}
        <div id="copyright">
            Powered by Sphinx <img src="img/sphinx.gif" alt="sphinx" /><br/>
            {{time}}s
        </div>
        {{end}}
        {{if powered_mysql}}
        <div id="copyright">
            Powered by MySQL <img src="img/mysql.gif" alt="mysql" /><br/>
            {{time}}s
        </div>
        {{end}}
        <br class="clear"/>
    </div>
    {{end}}
</body>
</html>