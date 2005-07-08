from ConfigParser import DEFAULTSECT, ConfigParser, NoSectionError, NoOptionError

section_defaults = { 'EXPERIMENTS' : { 'expdir_root' : '',
                                       'report_root' : ''
                                       }
                     }

def config_file( config_path = '.pyplearn' ):
    configuration_file = ConfigParser( )
    configuration_file.read( config_path )
    return configuration_file

def get_option( section, option ):
    try:
        return config_file( ).get( section, option )
    except (NoSectionError, NoOptionError), err:
        if section in section_defaults and \
           option in section_defaults[section]:
            return section_defaults[section][option]
        raise
