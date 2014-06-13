require 'pathname'
require 'fileutils'

app = ARGV[0]
qt = ARGV[1]

contents = {
  "Frameworks" => {
    "QtSvg.framework"          => { "Versions" => { "5" => ["@qt/lib/QtSvg.framework/QtSvg"] } },
    "QtWidgets.framework"      => { "Versions" => { "5" => ["@qt/lib/QtWidgets.framework/QtWidgets"] } },
    "QtGui.framework"          => { "Versions" => { "5" => ["@qt/lib/QtGui.framework/QtGui"] } },
    "QtCore.framework"         => { "Versions" => { "5" => ["@qt/lib/QtCore.framework/QtCore"] } },
    "QtMultimedia.framework"   => { "Versions" => { "5" => ["@qt/lib/QtMultimedia.framework/QtMultimedia"] } },
    "QtNetwork.framework"      => { "Versions" => { "5" => ["@qt/lib/QtNetwork.framework/QtNetwork"] } },
    "QtXml.framework"          => { "Versions" => { "5" => ["@qt/lib/QtXml.framework/QtXml"] } },
    "QtPrintSupport.framework" => { "Versions" => { "5" => ["@qt/lib/QtPrintSupport.framework/QtPrintSupport"] } }
  },
  "PlugIns" => {
    "accessible" => ["@qt/plugins/accessible/libqtaccessiblewidgets.dylib"],
    "imageformats" => [
      "@qt/plugins/imageformats/libqgif.dylib",
      "@qt/plugins/imageformats/libqico.dylib",
      "@qt/plugins/imageformats/libqjpeg.dylib",
      "@qt/plugins/imageformats/libqmng.dylib",
      "@qt/plugins/imageformats/libqtga.dylib",
      "@qt/plugins/imageformats/libqsvg.dylib"
    ],
    "platforms" => ["@qt/plugins/platforms/libqcocoa.dylib"],
    "printsupport" => ["@qt/plugins/printsupport/libcocoaprintersupport.dylib"],
    "audio" => ["@qt/plugins/audio/libqtaudio_coreaudio.dylib"],
    "iconengines" => ["@qt/plugins/iconengines/libqsvgicon.dylib"]
  }
}

class Deployer
  def initialize(appdir, qt, contents)
    @appdir = appdir
    @exec_path = executable(@appdir)
    raise "executable not found" if @exec_path.nil?
    @qt_path = qt
    @contents = contents
  end

  def binaries(dir)
    Dir.glob("#{dir}/**/*").select do |f|
      type = `file #{f}`
      type.include?("shared library") || type.include?("executable")
    end
  end

  def executable(dir)
    files = Dir.glob("#{dir}/**/*").select do |f|
      type = `file #{f}`
      type.include?("executable")
    end
    files[0]
  end

  def exe_relative_path(path)
    path.relative_path_from(Pathname.new(@exec_path).parent)
  end

  def visit_contents(c, path)
    c.each do |key, value|
      dir = path + key
      Dir.mkdir(dir) unless Dir.exists?(dir)

      if value.is_a? Hash
        visit_contents(value, dir)
      elsif value.is_a? Array
        value.each do |f|
          src = Pathname.new(f.sub("@qt", @qt_path))
          dst = dir + src.basename
          FileUtils.remove_entry_secure(dst) if File.exists?(dst)
          FileUtils.copy(src, dst)
        end
      end

    end
  end

  def run
    contents_path = Pathname.new(@exec_path).parent.parent
    visit_contents(@contents, contents_path)
    bins = {}
    binaries(@appdir).each do |b|
      bins[Pathname.new(b).basename] = Pathname.new(b)
    end
    bins.each do |k, b|
      `otool -L #{b}`.each_line do |l|
        lib = Pathname.new(l.gsub(/\(.+\)/, "").strip)
        if bins.key?(lib.basename)
          relative = exe_relative_path(bins[lib.basename])
          if b.basename == lib.basename
            `install_name_tool -id @executable_path/#{relative} #{b}`
          else
            `install_name_tool -change #{lib} @executable_path/#{relative} #{b}`
          end
        end
      end
    end
  end
end

d = Deployer.new(app, qt, contents)
d.run
