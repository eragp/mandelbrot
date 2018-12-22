import * as React from "react";
import BalancerPolicy from "../misc/BalancerPolicy";

// Stylesheet
import "./SelectBox.css";

// images
import mandelbrot from "./img/server.svg";

interface SelectBoxProps {
  balancer: BalancerPolicy;
}

interface BoxItem {
  key: string;
  title: string;
  description: string;
}
interface BoxItemList {
  active: string;
  title: string;
  items: BoxItem[];
}
interface SelectBoxState {
  balancer: BoxItemList;
  impl: BoxItemList;
}

export default class SelectBox extends React.Component<SelectBoxProps, SelectBoxState> {
  constructor(props: SelectBoxProps) {
    super(props);
    this.state = require("./SelectBoxOptions.json");
  }

  private setBalancer(val: string) {
    if (val === this.state.balancer.active) {
      return;
    }
    this.props.balancer.setBalancer(val);
    this.setState(prev => {
      return {
        balancer: Object.assign(prev.balancer, { active: val })
      };
    });
  }

  private setImpl(val: string) {
    if (val === this.state.balancer.active) {
      return;
    }
    // this.props.balancerPolicy.setBalancer(val);
    this.setState(prev => {
      return {
        impl: Object.assign(prev.impl, { active: val })
      };
    });
  }

  render() {
    return (
      <div className="row">
        <ItemBox onChange={this.setBalancer.bind(this)} {...this.state.balancer} />
        <ItemBox onChange={this.setImpl.bind(this)} {...this.state.impl} />
      </div>
    );
  }
}

interface ItemBoxProps extends BoxItemList {
  onChange: ((s: string) => void);
}
function ItemBox(list: ItemBoxProps) {
  return (
    <div className="select-box col">
      <h3 className="item-heading">{list.title}</h3>
      <div className="list-group">
        {list.items.map(it => (
          <Item
            key={it.key}
            handleClick={() => list.onChange(it.key)}
            active={it.key === list.active}
            {...it}
          />
        ))}
      </div>
    </div>
  );
}

interface ItemProps extends BoxItem {
  active: boolean;
  //   disabled: boolean;
  handleClick: ((e: any) => void);
}
interface ItemState {
  hovered: boolean;
}
class Item extends React.Component<ItemProps, ItemState> {
  constructor(props: ItemProps) {
    super(props);
    this.state = {
      hovered: false
    };
  }

  render() {
    let classList = "item list-group-item list-group-item-action flex-column ";
    if (this.props.active) {
      classList += "active";
    }
    // if (this.props.disabled) {
    //   classList += "disabled";
    // }
    return (
      <div
        className={classList}
        onClick={this.props.handleClick}
        onMouseEnter={() => this.setState({ hovered: true })}
        onMouseLeave={() => this.setState({ hovered: false })}
      >
        <h4>{this.props.title}</h4>
        <span className="helper" />
        <img src={mandelbrot} alt="" />
        <Description text={this.props.description} hovered={this.state.hovered} />
      </div>
    );
  }
}

interface DescriptionProps {
  text: string;
  hovered: boolean;
}
function Description(props: DescriptionProps) {
  let classList = "description ";
  classList += props.hovered ? "hover " : "";
  return <small className={classList}>{props.text}</small>;
}
