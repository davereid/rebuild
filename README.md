## Drush Rebuild

Drush Rebuild is a utility for rebuilding your local development environments,
using your existing Drush aliases and an easy-to-read `rebuild.info` file that
defines the tasks for rebuilding your local environment.

[![Build Status](https://travis-ci.org/kostajh/rebuild.png?branch=7.x-1.x)](https://travis-ci.org/kostajh/rebuild)

## Installation

Drush Rebuild is simple to install and configure:

  - Type `drush dl rebuild`, and Drush Rebuild will be installed in `~/.drush/rebuild`
  - Edit the Drush alias for the site you want to work on and specify a path to a rebuild config
  - Copy the example config file from `~/.drush/rebuild/examples` to the location you specified in #2.
  - Rebuild your local development environment! `drush rebuild @example.local --source=@example.prod`

## Need help?

Check out the [handbook page](https://drupal.org/node/1946954) for more
information.

## Commands

  - `drush rebuild @example.local --source=@example.prod` - Rebuild the environment at `@example.local` using the source `@example.prod`
  - `drush rebuild @example.local --view-config` - View the config for rebuilding `@example.local`. Does not execute any tasks.
  - `drush rebuild-info @example.local` - Displays statistics on rebuilds for `@example.local`

### Help

  - `drush help rebuild` - Get an overview of available commands and options.
  - `drush topic rebuild-readme` - Displays the README
  - `drush topic rebuild-example` - Displays an example rebuild script.

## Credits

- Kosta Harlan ([@kostajh](https://drupal.org/user/209141)) [development]
- Jay Roberts ([@GloryFish](https://drupal.org/user/281675)) [qa testing]
- Development sponsored by [DesignHammer Media Group](http://designhammer.com)
