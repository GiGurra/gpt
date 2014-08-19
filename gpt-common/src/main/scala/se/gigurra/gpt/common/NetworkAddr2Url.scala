package se.gigurra.gpt.common

import se.gigurra.gpt.model.common.NetworkTarget

object NetworkAddr2Url {
  def apply(a: NetworkTarget): String = {
	  s"ws://${a.getIp}:${a.getPort}"
	}
}