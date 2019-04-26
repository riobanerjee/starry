require=(function e(t,n,r){function s(o,u){if(!n[o]){if(!t[o]){var a=typeof require=="function"&&require;if(!u&&a)return a(o,!0);if(i)return i(o,!0);var f=new Error("Cannot find module '"+o+"'");throw f.code="MODULE_NOT_FOUND",f}var l=n[o]={exports:{}};t[o][0].call(l.exports,function(e){var n=t[o][1][e];return s(n?n:e)},l,l.exports,e,t,n,r)}return n[o].exports}var i=typeof require=="function"&&require;for(var o=0;o<r.length;o++)s(r[o]);return s})({"sphinx-rtd-theme":[function(require,module,exports){
var jQuery = (typeof(window) != 'undefined') ? window.jQuery : require('jquery');

// Sphinx theme nav state
function ThemeNav () {

    var nav = {
        navBar: null,
        win: null,
        winScroll: false,
        winResize: false,
        linkScroll: false,
        winPosition: 0,
        winHeight: null,
        docHeight: null,
        isRunning: null
    };

    nav.enable = function () {
        var self = this;

        jQuery(function ($) {
            self.init($);

            self.reset();
            self.win.on('hashchange', self.reset);

            // Set scroll monitor
            self.win.on('scroll', function () {
                if (!self.linkScroll) {
                    self.winScroll = true;
                }
            });
            setInterval(function () { if (self.winScroll) self.onScroll(); }, 25);

            // Set resize monitor
            self.win.on('resize', function () {
                self.winResize = true;
            });
            setInterval(function () { if (self.winResize) self.onResize(); }, 25);
            self.onResize();
        });
    };

    nav.init = function ($) {
        var doc = $(document),
            self = this;

        this.navBar = $('div.wy-side-scroll:first');
        this.win = $(window);

        // Set up javascript UX bits
        $(document)
            // Shift nav in mobile when clicking the menu.
            .on('click', "[data-toggle='wy-nav-top']", function() {
                $("[data-toggle='wy-nav-shift']").toggleClass("shift");
                $("[data-toggle='rst-versions']").toggleClass("shift");
            })

            // Nav menu link click operations
            .on('click', ".wy-menu-vertical .current ul li a", function() {
                var target = $(this);
                // Close menu when you click a link.
                $("[data-toggle='wy-nav-shift']").removeClass("shift");
                $("[data-toggle='rst-versions']").toggleClass("shift");
                // Handle dynamic display of l3 and l4 nav lists
                self.toggleCurrent(target);
                self.hashChange();
            })
            .on('click', "[data-toggle='rst-current-version']", function() {
                $("[data-toggle='rst-versions']").toggleClass("shift-up");
            })

        // Make tables responsive
        $("table.docutils:not(.field-list)")
            .wrap("<div class='wy-table-responsive'></div>");

        // Add expand links to all parents of nested ul
        $('.wy-menu-vertical ul').not('.simple').siblings('a').each(function () {
            var link = $(this);
                expand = $('<span class="toctree-expand"></span>');
            expand.on('click', function (ev) {
                self.toggleCurrent(link);
                ev.stopPropagation();
                return false;
            });
            link.prepend(expand);
        });
    };

    nav.reset = function () {
        // Get anchor from URL and open up nested nav
        var anchor = encodeURI(window.location.hash);
        if (anchor) {
            try {
                var link = $('.wy-menu-vertical')
                    .find('[href="' + anchor + '"]');
                $('.wy-menu-vertical li.toctree-l1 li.current')
                    .removeClass('current');
                link.closest('li.toctree-l2').addClass('current');
                link.closest('li.toctree-l3').addClass('current');
                link.closest('li.toctree-l4').addClass('current');
            }
            catch (err) {
                console.log("Error expanding nav for anchor", err);
            }
        }
    };

    nav.onScroll = function () {
        this.winScroll = false;
        var newWinPosition = this.win.scrollTop(),
            winBottom = newWinPosition + this.winHeight,
            navPosition = this.navBar.scrollTop(),
            newNavPosition = navPosition + (newWinPosition - this.winPosition);
        if (newWinPosition < 0 || winBottom > this.docHeight) {
            return;
        }
        this.navBar.scrollTop(newNavPosition);
        this.winPosition = newWinPosition;
    };

    nav.onResize = function () {
        this.winResize = false;
        this.winHeight = this.win.height();
        this.docHeight = $(document).height();
    };

    nav.hashChange = function () {
        this.linkScroll = true;
        this.win.one('hashchange', function () {
            this.linkScroll = false;
        });
    };

    nav.toggleCurrent = function (elem) {
        var parent_li = elem.closest('li');
        parent_li.siblings('li.current').removeClass('current');
        parent_li.siblings().find('li.current').removeClass('current');
        parent_li.find('> ul li.current').removeClass('current');
        parent_li.toggleClass('current');
    }

    return nav;
};

module.exports.ThemeNav = ThemeNav();

if (typeof(window) != 'undefined') {
    window.SphinxRtdTheme = { StickyNav: module.exports.ThemeNav };
}

// @rodluger: String trimmer
if(typeof(String.prototype.trim) === "undefined")
{
    String.prototype.trim = function() 
    {
        return String(this).replace(/^\s+|\s+$/g, '');
    };
}

// @rodluger: Create version dropdown
version_div = document.getElementsByClassName("version")[0];
var current_version = "v" + version_div.innerHTML.trim();
version_div.innerHTML = "";
version_selector = document.createElement("select");

// @rodluger: Read versions from github and add to dropdown
var txtFile = new XMLHttpRequest();
txtFile.open("GET", "https://raw.githubusercontent.com/rodluger/starry/gh-pages/versions.txt", true);
txtFile.onreadystatechange = function()
{
  if (txtFile.readyState === 4) {  // document is ready to parse.
    if (txtFile.status === 200) {  // file is found
      allText = txtFile.responseText; 
      versions = txtFile.responseText.split("\n");
      for (var i in versions){
            if (versions[i].length) {
                var op = new Option();
                op.value = versions[i];
                if (versions[i].trim() == current_version)
                    op.selected = true;
                else
                    op.selected = false;
                op.text = versions[i];
                version_selector.options.add(op); 
                version_div.appendChild(version_selector);
            }
      }

    }
  }
}
txtFile.send(null);

// @rodluger: Re-direct to the selected version's docs
version_selector.onchange = function()
{
    window.location.href = "/starry/" + this.value;
}

// @rodluger: Add parent commit info
var commit_url = "https://github.com/rodluger/starry/tree/571e087a9a07d9b8fe92d4ca5603c3f7199ac058";
var branch_url = "https://github.com/rodluger/starry/tree/dev";
var branch_name = "dev";
var d = new Date();
var months = ["January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"];
var full_date = months[d.getMonth()] + " " + d.getDate() + ", " + d.getFullYear();
if (branch_name != "unknown") {
    footer_div = document.getElementsByTagName("footer")[0];
    footer_div.innerHTML += 
        "Auto-generated on <a href='https://travis-ci.org'>Travis</a> from <a href='" + 
        commit_url + "'>this commit</a> on the <a href='" + 
        branch_url + "'>" + branch_name + "</a> branch on " + full_date + ".";
}

},{"jquery":"jquery"}]},{},["sphinx-rtd-theme"]);






















