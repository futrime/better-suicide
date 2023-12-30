# better-suicide

Allow players to suicide in Minecraft.

## Install

Use lip to install.

```powershell
lip install github.com/tooth-hub/better-suicide
```

## Usage

### Configurations

The configuration file under `config` directory in the plugin directory looks like:

```json
{
    "version": 1,
    "doGiveClockOnFirstJoin": true,
    "enableClockMenu": true
}
```

- `version`: format version of the configuration file. You shouldn't change it.
- `doGiveClockOnFirstJoin`: if true, players first joining the server will be given a clock (for menu entrypoint purpose).
- `enableClockMenu`: if true, when using a clock, a suicide confirmation menu will appear.

### How to Commit Suicide

Players can commit suicide by typing command `/suicide`. Players can also use a clock to commit suicide. But in this way, a confirmation menu pops up to prevent misuse.

## Contributing

PRs accepted.

This project follows the [Contributor Covenant](https://www.contributor-covenant.org/version/2/1/code_of_conduct/) Code of Conduct.

## License

LGPL-3.0-only © futrime
