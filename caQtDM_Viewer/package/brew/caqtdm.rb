class Caqtdm < Formula
  desc "caQtDM offers a collection of special widgets in order to display and control control system data in various ways"
  homepage "https://github.com/caqtdm/caqtdm"
  url "https://github.com/caqtdm/caqtdm/archive/refs/tags/V4.5.0.tar.gz"
  sha256 "153a3d7355a6f6412343b0e2cb876a3dafee06c97e3f1fdcb849320134ee6d1e"
  license "GPLv3"

  depends_on "qt"
  depends_on "qwt"
  depends_on "epicsbase"

  def install

    ENV["QTDIR"] = Formula["qt"].opt_prefix
    ENV.append_path "PATH", Formula["qt"].opt_bin
    ENV["EPICS_BASE"] = Formula["epicsbase"].opt_prefix
    ENV["EPICS_HOST_ARCH"] = "darwin-aarch64"
    ENV["EPICSINCLUDE"] = Formula["epicsbase"].opt_prefix
    ENV["EPICSINCLUDE"] += "/include"
    ENV["EPICSLIB"] = Formula["epicsbase"].opt_prefix
    ENV["EPICSLIB"] += "/lib/#{ENV["EPICS_HOST_ARCH"]}" 
    ENV["CAQTDM_MODBUS"] = "1"
    ENV["CAQTDM_GPS"] = "1"
    ENV["QTDIR"] = Formula["qt"].opt_prefix
    ENV["QTHOME"] = Formula["qt"].opt_prefix	
    ENV["QWTHOME"] = Formula["qwt"].opt_prefix
    ENV["CAQTDM_COLLECT"] = "#{prefix}"
    ENV["QTCONTROLS_LIBS"] = "#{prefix}"
    ENV["QWTVERSION"] = "6.1"
    ENV["QWTLIBNAME"] = "qwt"
    ENV["QWTLIB"] = Formula["qwt"].opt_prefix
    ENV["QWTLIB"] += "/lib" 
    #ENV["QWTINCLUDE"] = Formula["qwt"].opt_prefix 
    #ENV["QWTINCLUDE"] += "/include" 
    ENV["QWTINCLUDE"] = "/opt/homebrew/opt/qwt/lib/qwt.framework/Headers"

    puts ">> Detected QWTLIB: #{ENV["QWTLIB"]}"
    puts ">> Detected qwt: #{Formula["qwt"].opt_prefix}"
    
    ENV["SDKROOT"] = MacOS.sdk_for_formula(self).path

    os = OS.mac? ? "macx" : OS.kernel_name.downcase
    compiler = ENV.compiler.to_s.match?("clang") ? "clang" : "g++"

    system "qmake", "PREFIX=#{prefix} release -spec #{os}-#{compiler}"
    system "make"
    system "make", "install"
    if OS.mac?
     caqtdm_path = "#{prefix}/caQtDM.app"
     qt_bin = Formula["qt"].opt_bin
     #system qt_bin/"macdeployqt", caqtdm_path, "-verbose=2"
     app_bin = "#{prefix}/caQtDM.app/Contents/MacOS/caQtDM"
     frameworks = "#{prefix}/caQtDM.app/Contents/Frameworks"
     plugins =  "#{prefix}/caQtDM.app/Contents/PlugIns/controlsystems"
     design =  "#{prefix}/caQtDM.app/Contents/PlugIns/designer"

     system "install_name_tool", "-id", "@rpath/libcaQtDM_Lib.dylib", "#{frameworks}/libcaQtDM_Lib.dylib"
     system "install_name_tool", "-id", "@rpath/libqtcontrols.dylib", "#{frameworks}/libqtcontrols.dylib"

     #system "install_name_tool", "-add_rpath", "@executable_path/../Frameworks", app_bin
     system "install_name_tool", "-change", "libcaQtDM_Lib.dylib", "@rpath/libcaQtDM_Lib.dylib", app_bin
     system "install_name_tool", "-change", "libqtcontrols.dylib", "@rpath/libqtcontrols.dylib", app_bin

     system "install_name_tool", "-change", "libcaQtDM_Lib.dylib", "@rpath/libcaQtDM_Lib.dylib", "#{plugins}/libepics3_plugin.dylib"
     
     system "install_name_tool", "-change", "libcaQtDM_Lib.dylib", "@rpath/libcaQtDM_Lib.dylib", "#{plugins}/libarchiveSF_plugin.dylib"
     system "install_name_tool", "-change", "libqtcontrols.dylib", "@rpath/libqtcontrols.dylib", "#{plugins}/libarchiveSF_plugin.dylib"      
     system "install_name_tool", "-change", "libcaQtDM_Lib.dylib", "@rpath/libcaQtDM_Lib.dylib", "#{plugins}/libarchiveHTTP_plugin.dylib"
     system "install_name_tool", "-change", "libqtcontrols.dylib", "@rpath/libqtcontrols.dylib", "#{plugins}/libarchiveHTTP_plugin.dylib"      

     system "install_name_tool", "-change", "libcaQtDM_Lib.dylib", "@rpath/libcaQtDM_Lib.dylib", "#{plugins}/libdemo_plugin.dylib"
     system "install_name_tool", "-change", "libqtcontrols.dylib", "@rpath/libqtcontrols.dylib", "#{plugins}/libdemo_plugin.dylib"

     system "install_name_tool", "-change", "libcaQtDM_Lib.dylib", "@rpath/libcaQtDM_Lib.dylib", "#{plugins}/libenvironment_plugin.dylib"
     system "install_name_tool", "-change", "libqtcontrols.dylib", "@rpath/libqtcontrols.dylib", "#{plugins}/libenvironment_plugin.dylib"

     system "install_name_tool", "-change", "libcaQtDM_Lib.dylib", "@rpath/libcaQtDM_Lib.dylib", "#{plugins}/libmodbus_plugin.dylib"
     system "install_name_tool", "-change", "libqtcontrols.dylib", "@rpath/libqtcontrols.dylib", "#{plugins}/libmodbus_plugin.dylib"

     system "install_name_tool", "-change", "libcaQtDM_Lib.dylib", "@rpath/libcaQtDM_Lib.dylib", "#{plugins}/libgps_plugin.dylib"
     system "install_name_tool", "-change", "libqtcontrols.dylib", "@rpath/libqtcontrols.dylib", "#{plugins}/libgps_plugin.dylib"

     system "install_name_tool", "-change", "libqtcontrols.dylib", "@rpath/libqtcontrols.dylib", "#{design}/libqtcontrols_controllers_plugin.dylib"
     system "install_name_tool", "-change", "libqtcontrols.dylib", "@rpath/libqtcontrols.dylib", "#{design}/libqtcontrols_graphics_plugin.dylib"
     system "install_name_tool", "-change", "libqtcontrols.dylib", "@rpath/libqtcontrols.dylib", "#{design}/libqtcontrols_monitors_plugin.dylib"
     system "install_name_tool", "-change", "libqtcontrols.dylib", "@rpath/libqtcontrols.dylib", "#{design}/libqtcontrols_utilities_plugin.dylib"





    end



  end

  test do
    # Optional: Ein einfacher Test, ob das Binary da ist
    assert_predicate bin/"caQtDM.app", :exist?
  end
end
