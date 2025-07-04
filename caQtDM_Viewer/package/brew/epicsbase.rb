class Epicsbase < Formula
  desc "EPICS Base - Experimental Physics and Industrial Control System"
  homepage "https://epics-controls.org/"
  url "https://github.com/epics-base/epics-base/archive/refs/tags/R7.0.9.tar.gz"
  sha256 "521ca482c7dbc55cc380b2fd4d160ef96a4169517a5e55ea8651cd2880da45ae"
  license "EPICS"

  depends_on "pkg-config" => :build
  depends_on "readline"
  depends_on "perl"

  def install
    # EPICS erwartet, dass diese Umgebungsvariablen gesetzt sind
    ENV["EPICS_HOST_ARCH"] = `./startup/EpicsHostArch`.strip
    ENV["EPICS_BASE"] = buildpath

    # Optional: Anpassung der Konfigurationsdateien
    inreplace "configure/CONFIG_SITE", /#?INSTALL_LOCATION=.*/, "INSTALL_LOCATION=#{prefix}"
    

    system "make"

    # Installation: einfach alles kopieren
    prefix.install Dir["*"]
  end

  def caveats
    <<~EOS
      EPICS Base wurde installiert.

      Um EPICS korrekt zu verwenden, füge dies zu deiner Shell-Konfiguration hinzu:

        export EPICS_BASE=#{opt_prefix}
        export EPICS_HOST_ARCH=$(#{opt_prefix}/startup/EpicsHostArch)

    EOS
  end

  test do
    # einfacher Test, ob z.B. caput installiert ist
    assert_predicate bin/"caput", :exist?
  end
end
