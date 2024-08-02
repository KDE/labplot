function get_files
{
    echo labplot.xml
}

function po_for_file
{
    case "$1" in
       labplot.xml)
           echo labplot_xml_mimetypes.po
       ;;
    esac
}

function tags_for_file
{
    case "$1" in
       labplot.xml)
           echo comment
       ;;
    esac
}
