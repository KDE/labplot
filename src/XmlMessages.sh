function get_files
{
    echo labplot2.xml
}

function po_for_file
{
    case "$1" in
       labplot2.xml)
           echo labplot2_xml_mimetypes.po
       ;;
    esac
}

function tags_for_file
{
    case "$1" in
       labplot2.xml)
           echo comment
       ;;
    esac
}
